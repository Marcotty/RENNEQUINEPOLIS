/**
 * split.cpp
 *
 * DESCRIPTION
 *
 * This sample code is intended to students of the SGBD Module 3 course.
 * It is provided as a sample solution to the exercise named Rennequinepolis.
 *
 * The application receives some text stream (typically the contents of file
 * "movies.txt", an off-standard delimited text file) through the standard
 * input stream (stdin). It parses every line and outputs a series of SQL
 * INSERT instructions, according to the database system type, which is passed
 * as an argument on the command line.
 *
 * Each input line represents a complete movie record from IMDB.com. Field
 * values are separated by a unicode character:
 *
 * - "\u2023" delimits first-order fields, i.e. movie fields
 * - "\u2016" delimits second-order records, i.e. movie genres or actors
 * - "\u2024" delimits second-order record fields
 *
 * The purpose of this sample code is to split every input line and output a
 * series of SQL statements for movies, directors and the cast of characters.
 *
 * \note MySQL SET and PostgreSQL array of ENUM's are supported for the genre
 * field, i.e. movie genre sub-records are converted and reduced to only the
 * genre name.
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

#include <stdlib.h>			// For EXIT_(SUCCESS|FAILURE)
#include <iostream>
#include <iomanip>			// For std::quoted()
#include <cstring>
#include <vector>
#include <algorithm>		// For copy/copy_if
#include "iterators.h"

static_assert(__cplusplus >= 201703L, "compile option -std=c++17 is required.");

// Separators
static constexpr char DOUBLE_VLINE[] = "\u2016";
static constexpr char TRIANGLE_BULLET[] = "\u2023";
static constexpr char DOT_LEADER[] = "\u2024";

/**
 * \brief Quote a string for database insertion
 *
 * The STL provides a generic way to quote strings while escaping the quote
 * sign. Default database quoting doubles the quote mark, a single quote char.
 */
template <typename T>
auto quote(T& value)
{ return std::quoted(value, '\'', '\''); }

/**
 * \brief Field delimiter
 *
 * This STL type alias is intended for use with UTF-8 field delimiters, whereas
 * usual delimiters generally are made of a single character. C/C++ store utf8
 * data with more bytes than the actual unicode length, hence the additional
 * length field in the delimiter structure.
 */
typedef std::string_view delimiter;

/**
 * \brief Raw record field iterator
 *
 * This forward-only iterator returns a string view from a raw record string
 * on every invocation.
 *
 * Note: This is not a fully STL-compliant iterator.
 */
struct splitter
{
	// Raw record view and delimiter references
	const std::string_view record;
	const delimiter& mark;

	// Field value start and end positions
	std::size_t begin, end;

	// Build an iterator for the given line and delimiter
	constexpr splitter(std::string_view, const delimiter&);

	// Return a string view using the current markers
	constexpr std::string_view operator * () const
	{ return { &record[begin], end-begin }; }

	// Increment operators
	constexpr splitter& operator ++ ();		// Pre-increment
	constexpr splitter operator ++ (int);	// Post-increment
};

inline constexpr splitter::splitter(std::string_view _s, const delimiter& _delimiter) :
		record(_s), mark(_delimiter),
		begin(0), end(std::min(record.find(mark), record.size())) {}

inline constexpr splitter& splitter::operator ++ ()
{
	// Don't search past the end of the record string
	const std::size_t length = record.length();
	if (begin < length)
	{
		const std::size_t next_stop = std::min(end + mark.size(), length);
		end = std::min(record.find(mark, begin = next_stop), length);
	}
	return *this;
}

inline constexpr splitter splitter::operator ++ (int)
{
	splitter tmp = *this;
	operator ++ ();
	return tmp;
}


constexpr auto raw_movie_fields = {
	"id", "title", "original_title", "release_date", "status",
	"vote_average", "vote_count", "runtime", "certification",
	"poster_path", "budget", "tag_line", "genre", "directors", "cast"
};

constexpr auto raw_genre_fields = {
	"id", "name"
};

constexpr auto raw_actor_fields = {
	"id", "actor_name", "character_name"
};

constexpr auto raw_director_fields = {
	"id", "director_name"
};

static const delimiter movie_delimiter(TRIANGLE_BULLET);
static const delimiter record_delimiter(DOUBLE_VLINE);
static const delimiter value_delimiter(DOT_LEADER);

/**
 * \brief Generic record
 *
 * A record contains a line string and a list of fields, the latter of which
 * are built when the record is constructed. The field list is mutable for
 * the sole sake of simplicity. The same record object is reused to hold the
 * split fields from a movie line.
 */
struct record
{
	typedef std::string::value_type char_type;
	typedef std::initializer_list<const char_type* const> name_list;

	// Record field aka key/value pair
	struct field
	{
		typedef const char_type* name_type;
		typedef std::string_view value_type;

		name_type name;
		value_type value;

		field() = default;
		field(const char_type* _name) : name(_name) {}
		field(const char_type* _name, const value_type& _value) :
			name(_name), value(_value) {}

		bool is_empty() const { return value.size() == 0; }
		operator bool () const { return !is_empty(); }

		operator name_type () const { return name; }
		operator value_type () const { return value; }
	};

	// Range of record fields
	template <typename T>
	struct range
	{
		const field *start, *end;
		typedef T value_type;
	};


	// Name & value ranges, used by output stream iterators
	typedef range<field::name_type> name_range;
	typedef range<field::value_type> value_range;

	// List of field names and markers
	std::vector<field> fields;

	// Construct a blank record using a field name list
	record(const name_list& list) :
		fields(list.begin(), list.end()) {}

	// Construct an initialized record
	record(std::initializer_list<field>&& list) : fields(list) {}

	// Return a range of iterators for field names
	static name_range names(const record&, int first = 0);
	static name_range names(const record&, int first, int last);

	// Return a range of value iterators
	static value_range values(const record&, int first = 0);
	static value_range values(const record&, int first, int last);

	// Return the number of fields in this record
	std::size_t size() const { return fields.size(); }

	// Split the record line into the expected field values
	std::size_t parse(const std::string_view&, const delimiter&);

	// Field access shortcut
	const field& operator [] (int index) const
	{ return fields[index < 0 ? fields.size() + index : index]; }

	field& operator [] (int index)
	{ return fields[index < 0 ? fields.size() + index : index]; }
};

std::size_t record::parse(const std::string_view& _str, const delimiter& _delimiter)
{
	splitter iterator(_str, _delimiter);
	for (auto& field: fields) field.value = *iterator++;
	return fields.size();
}

inline record::name_range record::names(const record& r, int first)
{ return names(r, first, r.size()); }

inline record::name_range record::names(const record& r, int first, int last)
{ return { &r[first], &r[last] }; }

inline record::value_range record::values(const record& r, int first)
{ return values(r, first, r.size()); }

inline record::value_range record::values(const record& r, int first, int last)
{ return { &r[first], &r[last] }; }


/**
 * \brief Record view, aka read-only record slice
 *
 * This class provides a read-only slice, i.e. a range of fields from a record
 * in a way similar to std::string_view. Record views are used to print out
 * entire records or a range of contiguous fields.
 *
 * Instances of this class are trivially copyable hence shouldn't need copy
 * or move constructors.
 */
struct record_view
{
	typedef record::name_range name_range;
	typedef record::value_range value_range;

	const record& view;
	int begin, end;

	name_range names() const { return record::names(view, begin, end); }
	value_range values() const { return record::values(view, begin, end); }

	record_view(const record& r, int first = 0) :
		view(r), begin(first), end(r.size()) {}

	constexpr record_view(const record& r, int first, int last) :
		view(r), begin(first), end(last) {}
};


/**
 * \brief Quote field values when printed out
 *
 * The custom function \c quote calls std::quoted internally to use single
 * quote marks and escape single quotes by doubling them.
 */
inline std::ostream& operator << (std::ostream& os, const record::field::value_type& v)
{ return os << quote(v); }

/**
 * \brief Output a range of names or values
 *
 * Use this external serialization operator to output a specific range of names
 * from a given record. Internally uses a custom output stream iterator that
 * joins every value with commas.
 */
template <typename T>
inline std::ostream& operator << (std::ostream& os, const record::range<T>& range)
{
	std::copy_if(range.start, range.end,
		ostream_iterator<T>(os, ", "),
		[](const record::field& f) { return !f.is_empty(); }
	);
	return os;
}

/**
 * \brief Database syntax interface
 *
 * This class defines an interface to output SQL instructions. Derivatives
 * include MySQL and PostgreSQL syntaxes.
 *
 * The required operations are:
 *
 * - The USE clause
 * - A field list formatter (e.g. MySQL SET, implemented as arrays in Postgres)
 * - An SQL INSERT statement operation for a given record view
 *
 * Note: derived classes are hereby non-trivial as they contain a virtual table.
 */
struct DB
{
	// Line terminator
	static constexpr const char* const endl = ";\n";

	// I/O stream to output SQL statements to
	std::ostream& out;

	DB(std::ostream& os) : out(os) {}

	virtual void use(const char* db_name) = 0;
	virtual std::string list(const std::string&) = 0;
	virtual void insert(const char* table, const record_view&) = 0;
};

/// MySQL database formatter
struct MySQL : DB
{
	MySQL(std::ostream& os) : DB(os) {}

	void use(const char* db_name);
	std::string list(const std::string&);
	void insert(const char* table, const record_view&);
};

void MySQL::use(const char* db_name)
{
	if (db_name) out << "USE " << db_name << endl;
}

std::string MySQL::list(const std::string& set)
{
	// NOOP: a MySQL set is exactly the list of comma-separated values and
	// quoted as a string value for insertion
	return set;
}

/// Map a record view to an SQL insertion (MySQL)
void MySQL::insert(const char* table, const record_view& view)
{
	out << "INSERT IGNORE " << table << '('
		<< view.names() << ')'
		<< " VALUES (" << view.values() << ')'
		<< endl;
}


/// PostgreSQL database formatter
struct PostgreSQL : DB
{
	PostgreSQL(std::ostream& os) : DB(os) {}

	void use(const char* db_name);
	std::string list(const std::string&);
	void insert(const char* table, const record_view&);
};

void PostgreSQL::use(const char* db_name)
{
	if (db_name) out << "USE " << db_name << endl;
}

std::string PostgreSQL::list(const std::string& set)
{
	// Postgres: Enclose arrays of enums with braces
	return '{' + set + '}';
}

/// Map a record view to an SQL insertion (PostgreSQL)
void PostgreSQL::insert(const char* table, const record_view& view)
{
	out << "INSERT INTO " << table << '('
		<< view.names() << ')'
		<< " VALUES (" << view.values() << ") ON CONFLICT DO NOTHING"
		<< endl;
}


/// Build a string of genres as a comma-separated list of values
std::string genres(const record::field& f)
{
	// Use a stream iterator to join every value sent to it with a comma
	std::stringstream buffer;
	ostream_iterator<std::string_view> out(buffer, ",");

	// For each genre sub-record append the text field to the list
	splitter in(f, record_delimiter);
	record movie_genre(raw_genre_fields);
	while (in.begin < in.record.size())
	{
		movie_genre.parse(*in++, value_delimiter);
		*out++ = movie_genre[1];
	}

	std::string set;
	buffer >> quote(set);

	return set;
}

int main(int argc, char **argv)
{
	try
	{
		// Select between MySQL & PostgreSQL
		if (argc < 2 || argc > 3) throw std::system_error(EINVAL, std::generic_category());

		std::unique_ptr<DB> db;
		if (std::string(argv[1]) == "--mysql") db = std::make_unique<MySQL>(std::cout);
		if (std::string(argv[1]) == "--postgres") db = std::make_unique<PostgreSQL>(std::cout);

		if (!db) throw std::runtime_error("database type is unspecified.");

		// Get and output the database name if any
		const char* db_name = argc == 3 ? argv[2] : nullptr;
		db->use(db_name);

		// Parse each line from the input stream
		std::string line;
		record movie(raw_movie_fields);
		while (std::getline(std::cin, line))
		{
			// Split raw record into raw fields
			movie.parse(line, movie_delimiter);

			// Split and replace the genre field (unquoting prevously quoted values)
			std::string tmp = db->list(genres(movie[-3]));
			movie[-3].value = tmp;

			// 1. insert the constructed movie record
			// Reuse all fields but the last 2 (directors & cast)
			db->insert("movies", record_view(movie, 0, -2));

			// This is also valid:
			// mysql_insert("movies", {movie, 0, -2});

			// 2. Build the director record(s)
			record director(raw_director_fields);
			for (splitter it(movie[-2], record_delimiter); it.begin < it.record.size();)
			{
				director.parse(*it++, value_delimiter);

				// Insert into people (the record instance is converted into a
				// record_view by the latter's constructor)
				db->insert("people", record({
					{ "id", director[0].value },
					{ "full_name", director[1].value },
				}));

				// Then insert into directors
				db->insert("directors", record({
					{ "movie_id", movie[0].value },
					{ "director_id", director[0].value },
				}));
			}

			// 3. Build the actor records
			record actor(raw_actor_fields);
			for (splitter it(movie[-1], record_delimiter); it.begin < it.record.size();)
			{
				actor.parse(*it++, value_delimiter);

				// Insert into people...
				db->insert("people", record({
					{ "id", actor[0].value },
					{ "full_name", actor[1].value },
				}));

				// ... then characters
				db->insert("characters", record({
					{ "movie_id", movie[0].value },
					{ "actor_id", actor[0].value },
					{ "character_name", actor[2].value },
				}));
			}
		}

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		std::cerr
			<< argv[0] << " : " << e.what()
			<< "\n\nSyntax: " << argv[0] << " --mysql|--postgres [DATABASE]\n";
		return EXIT_FAILURE;
	}
}
