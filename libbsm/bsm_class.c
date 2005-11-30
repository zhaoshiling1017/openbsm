/*
 * Copyright (c) 2004, Apple Computer, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bsm/libbsm.h>

#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Parse the contents of the audit_class file to return struct au_class_ent
 * entries.
 */
static FILE	*fp = NULL;
static char	linestr[AU_LINE_MAX];
static char	*delim = ":";

static pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * XXX The reentrant versions of the following functions is TBD
 * XXX struct au_class_ent *getclassent_r(au_class_ent_t *class_int);
 * XXX struct au_class_ent *getclassnam_r(au_class_ent_t *class_int, const
 *         char *name);
 */

/*
 * Allocate a au_class_ent structure.
 */
static struct au_class_ent *
get_class_area(void)
{
	struct au_class_ent *c;

	c = malloc(sizeof(struct au_class_ent));
	if (c == NULL)
		return (NULL);
	c->ac_name = malloc(AU_CLASS_NAME_MAX * sizeof(char));
	if (c->ac_name == NULL) {
		free(c);
		return (NULL);
	}
	c->ac_desc = malloc(AU_CLASS_DESC_MAX * sizeof(char));
	if (c->ac_desc == NULL) {
		free(c->ac_name);
		free(c);
		return (NULL);
	}

	return (c);
}


/*
 * Free the au_class_ent structure.
 */
void
free_au_class_ent(struct au_class_ent *c)
{

	if (c) {
		if (c->ac_name)
			free(c->ac_name);
		if (c->ac_desc)
			free(c->ac_desc);
		free(c);
	}
}

/*
 * Parse a single line from the audit_class file passed in str to the struct
 * au_class_ent elements; store the result in c.
 */
static struct au_class_ent *
classfromstr(char *str, char *delim, struct au_class_ent *c)
{
	char *classname, *classdesc, *classflag;
	char *last;

	/* Each line contains flag:name:desc. */
	classflag = strtok_r(str, delim, &last);
	classname = strtok_r(NULL, delim, &last);
	classdesc = strtok_r(NULL, delim, &last);

	if ((classflag == NULL) || (classname == NULL) || (classdesc == NULL))
		return (NULL);

	/*
	 * Check for very large classnames.
	 */
	if (strlen(classname) >= AU_CLASS_NAME_MAX)
		return (NULL);

	strcpy(c->ac_name, classname);

	/*
	 * Check for very large class description.
	 */
	if (strlen(classdesc) >= AU_CLASS_DESC_MAX)
		return (NULL);
	strcpy(c->ac_desc, classdesc);
	c->ac_class = strtoul(classflag, (char **) NULL, 0);

	return (c);
}

/*
 * Return the next au_class_ent structure from the file setauclass should be
 * called before invoking this function for the first time.
 *
 * Must be called with mutex held.
 */
static struct au_class_ent *
getauclassent_locked(void)
{
	struct au_class_ent *c;
	char *tokptr, *nl;

	if ((fp == NULL) && ((fp = fopen(AUDIT_CLASS_FILE, "r")) == NULL))
		return (NULL);

	/*
	 * Read until next non-comment line is found, or EOF.
	 */
	while (1) {
		if (fgets(linestr, AU_LINE_MAX, fp) == NULL)
			return (NULL);
		if (linestr[0] != '#')
			break;
	}

	/* Remove trailing new line character. */
	if ((nl = strrchr(linestr, '\n')) != NULL)
		*nl = '\0';

	tokptr = linestr;

	c = get_class_area(); /* allocate */
	if (c == NULL)
		return (NULL);

	/* Parse tokptr to au_class_ent components. */
	if (classfromstr(tokptr, delim, c) == NULL) {
		free_au_class_ent(c);
		return (NULL);
	}

	return (c);
}

struct au_class_ent *
getauclassent(void)
{
	struct au_class_ent *c;

	pthread_mutex_lock(&mutex);
	c = getauclassent_locked();
	pthread_mutex_unlock(&mutex);
	return (c);
}

/*
 * Rewind to the beginning of the enumeration.
 *
 * Must be called with mutex held.
 */
static void
setauclass_locked(void)
{

	if (fp != NULL)
		fseek(fp, 0, SEEK_SET);
}

void
setauclass(void)
{

	pthread_mutex_lock(&mutex);
	setauclass_locked();
	pthread_mutex_unlock(&mutex);
}

/*
 * Return the next au_class_entry having the given class name.
 */
au_class_ent_t *
getauclassnam(const char *name)
{
	struct au_class_ent *c;

	if (name == NULL)
		return (NULL);

	pthread_mutex_lock(&mutex);
	setauclass_locked();
	while ((c = getauclassent()) != NULL) {
		if (strcmp(name, c->ac_name) == 0) {
			pthread_mutex_unlock(&mutex);
			return (c);
		}
		free_au_class_ent(c);
	}
	pthread_mutex_unlock(&mutex);
	return (NULL);
}

/*
 * Return the next au_class_entry having the given class number.
 *
 * OpenBSM extension.
 */
au_class_ent_t *
getauclassnum(au_class_t class_number)
{
	au_class_ent_t *c;

	pthread_mutex_lock(&mutex);
	setauclass_locked();
	while ((c = getauclassent()) != NULL) {
		if (class_number == c->ac_class)
			return (c);
		free_au_class_ent(c);
	}
	pthread_mutex_unlock(&mutex);
	return (NULL);
}

/*
 * audit_class processing is complete; close any open files.
 */
void endauclass(void)
{

	pthread_mutex_lock(&mutex);
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	pthread_mutex_unlock(&mutex);
}
