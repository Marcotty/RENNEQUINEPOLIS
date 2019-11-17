/**
 * csplit.c
 *
 * DESCRIPTION
 *
 * \see split.cpp
 *
 * LICENSING
 *
 * Copyright 2019 Vincent Cadet <vincent.cadet@hepl.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

// We want getline() to dynamically realloc() the line buffer automatically
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Array utilities
#define FIELD_COUNT(array)	(sizeof(array) / sizeof(array[0]))

// UTF-8 delimiters
static const char TRIANGLE_BULLET[] = "\u2023";
static const char DOUBLE_VLINE[] = "\u2016";
static const char DOT_LEADER[] = "\u2024";

// Field value: a pair of (start, length) markers
typedef struct
{
	const char* begin;
	size_t length;
} marker_type;

// Field definition: a name and a marker
typedef struct
{
	const char* name;
	marker_type marker;
} field;

// Field manipulators (using C99 "inline" syntax)
static inline size_t fldlen(const field* fld)
{ return fld->marker.length; }

static inline const char* fldbegin(const field* fld)
{ return fld->marker.begin; }

static inline const char* fldend(const field* fld)
{ return &fld->marker.begin[fld->marker.length]; }

static inline void print_name(const field* fld)
{ printf("%s", fld->name); }

static inline void print_value(const field* fld)
{ printf("%.*s", (int)fld->marker.length, fld->marker.begin); }

// The main record
static field raw_movie[] = {
	{ "id" },
	{ "title" },
	{ "original_title" },
	{ "release_date" },
	{ "status" },
	{ "vote_average" },
	{ "vote_count" },
	{ "runtime" },
	{ "certification" },
	{ "poster_path" },
	{ "budget" },
	{ "tag_line" },
	{ "genre" },
	{ "directors" },
	{ "cast" },
};

static field raw_genre[] = {
	{ "genre_id" },
	{ "genre_name" },
};

static field raw_director[] = {
	{ "director_id" },
	{ "director_name" },
};

static field raw_character[] = {
	{ "actor_id" },
	{ "actor_name" },
	{ "character_name" },
};

// Field identifiers: movies, genres, directors & characters
enum {
	MOVIE_ID, TITLE, ORIGINAL_TITLE, RELEASE_DATE, STATUS,
	VOTE_AVERAGE, VOTE_COUNT, RUNTIME, CERTIFICATION,
	POSTER_PATH, BUDGET, TAG_LINE, GENRE, DIRECTORS, CAST
};

enum { GENRE_ID, GENRE_NAME };

enum { DIRECTOR_ID, DIRECTOR_NAME };

enum { ACTOR_ID, ACTOR_NAME, CHARACTER_NAME };

// Break a raw movie line into fields with a given delimiter
int parse_record(const char* line, const char* end,
				 const char* delim, field* fields, size_t field_count)
{
	// Arguments must make sense!
	if (line == NULL || fields == NULL || field_count == 0) return -1;

	const char* stop;
	const size_t delimiter_length = strlen(delim);

	do {
		// Find next delimiter
		stop = strstr(line, delim);

		// Don't look past the end of the string
		if (!stop || stop > end) stop = end;

		// Capture the current field
		fields->marker.begin = line;
		fields->marker.length = stop - line;

		// Prepare next field
		if (--field_count == 0 || stop > end) break;
		line = stop + delimiter_length;
		fields++;
	} while (stop < end);

	return 0;
}

const char* fetch_record(const char* begin, const char* end,
						 field* record, size_t field_count)
{
	// Lookup next record delimiter but don't go beyond the end of the field
	const char* stop = strstr(begin, DOUBLE_VLINE);
	if (!stop || stop > end) stop = end;

	// Extract genre fields
	parse_record(begin, stop, DOT_LEADER, record, field_count);

	// Set new start
	// TODO: Cache delimiter length
	return stop + strlen(DOT_LEADER);
}

// Escape and quote a value, to be printed on a per-character basis
void sql_quote(const char* str, size_t length)
{
	if (length == 0) printf("NULL");
	else if (str)
	{
		putchar('\'');
		while (length--)
		{
			// Escape: double the quote
			if (*str == '\'') putchar(*str);
			putchar(*str++);
		}
		putchar('\'');
	}
}

// Build an SQL INSERT statement for a given record
void mysql_insert(const char* table_name, field* fields, int count)
{
	// Output names
	printf("INSERT IGNORE %s (", table_name);
	for (size_t i = 0; i < count; i++)
		printf(i ? ", %s" : "%s", fields[i].name);

	printf(") VALUES (");

	// Output values, escaping & quoting them
	for (size_t i = 0; i < count; i++)
	{
		if (i) fprintf(stdout, ", ");
		sql_quote(fields[i].marker.begin, fldlen(&fields[i]));
	}

	printf(");\n");
}

/* Split every incoming line into movie fields and build SQL INSERT statements.
 * Only MySQL is supported. */
int main(int argc, char **argv)
{
	// More than one argument is an error
	if (argc > 2)
	{
		fprintf(stderr, "Too many arguments.\n\nUsage: %s [database]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Database argument is optional
	if (argc == 2) printf("USE %s;\n", argv[1]);

	// Buffer pointer and size MUST be initialized to zero for getline()
	char* line_buffer = NULL;
	size_t buffer_size = 0;

	// Buffer to be reallocated for movie genres
	char* genres = NULL;
	size_t current_size = 0;

	// Use a symbolic name for buffered output (stdin by default)
	FILE* input = stdin;

	// Repeatedly fetch lines from the input stream until nothing new or an
	// error occurs, in which case getline() returns -1
	ssize_t line_length;
	while((line_length = getline(&line_buffer, &buffer_size, input)) > 0)
	{
		// Parse the raw movie record completely
		parse_record(line_buffer, &line_buffer[--line_length], TRIANGLE_BULLET,
					 raw_movie, FIELD_COUNT(raw_movie));

		// Pick the current size of the genre fields as a base as there won't
		// be more characters than what the record list counts: ID and field
		// separator are strip and replced with a comma. Grow memory usage
		// for the target genre string whenever appropriate though.
		if (fldlen(&raw_movie[GENRE]) > current_size)
			genres = (char*)realloc(genres, current_size = fldlen(&raw_movie[GENRE]));

		// Pack genres from the genre record list
		if (genres)
		{
			// Reset genre list
			genres[0] = '\0';

			// Fetch movie genres
			const char* record_end = fldend(&raw_movie[GENRE]);
			const char* current_record = fldbegin(&raw_movie[GENRE]);
			while (current_record < record_end)
			{
				// Iterate through the genre list
				current_record = fetch_record(current_record, record_end,
					raw_genre, FIELD_COUNT(raw_genre));

				// Append new genre with a separator if at least one exists
				if (genres[0]) strcat(genres, ",");
				strncat(genres, fldbegin(&raw_genre[GENRE_NAME]), fldlen(&raw_genre[GENRE_NAME]));
			}

			// Assign genre list: replace the current record markers
			raw_movie[GENRE].marker.begin = genres;
			raw_movie[GENRE].marker.length = strlen(genres);
		}

		// Insert current movie (up to but excluding the directors field)
		mysql_insert("movies", raw_movie, DIRECTORS);

		// Director field is left as an exercise
		// TODO: Insert directors...

		// Fetching actors
		if (fldlen(&raw_movie[CAST]))
		{
			const char* record_end = fldend(&raw_movie[CAST]);
			const char* current_record = fldbegin(&raw_movie[CAST]);
			while (current_record < record_end)
			{
				// Iterate through the character list
				current_record = fetch_record(current_record, record_end,
					raw_character, FIELD_COUNT(raw_character));

				// Insert into people first
				field person[] = {
					{ "id",				raw_character[ACTOR_ID].marker },
					{ "full_name",		raw_character[ACTOR_NAME].marker },
				};
				mysql_insert("people", person, FIELD_COUNT(person));

				// ... then cast
				field character[] = {
					{ "movie_id",		raw_movie[MOVIE_ID].marker },
					{ "actor_id",		raw_character[ACTOR_ID].marker },
					{ "character_name",	raw_character[CHARACTER_NAME].marker },
				};
				mysql_insert("characters", character, FIELD_COUNT(character));
			}
		}
	}

	// Free the pre-allocated buffers
	if (buffer_size) free(line_buffer);
	if (current_size) free(genres);

	return EXIT_SUCCESS;
}
