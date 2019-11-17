#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  db.py
#
#  Copyright 2019 Vincent Cadet <vincent.cadet@hepl.be>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

"""
Module db.py : fonctions basiques d'importation de données. Les systèmes pris
en charge sont:

 * MySQL
 * PostgreSQL
"""

def escape(v):
	""" Escape single quotes by just doubling them as per SQL requirements """
	return v.replace("'", "''")

def quote(v):
	""" Quote a single value """
	return "'{}'".format(v)

def filter_record(record_tuple):
	""" Return a dictionary from a named tuple with only those properties
	which evaluate to a non-empty string """
	return { k: quote(escape(v)) for (k, v) in record_tuple._asdict().items() if v.strip() }

def write(*args, **kwargs):
	""" Write all arguments, terminating each line with a semicolon """
	print(*args, end=';\n', **kwargs)


class MySQL(object):
	def __init__(self, db_name):
		write('USE', db_name)

	def list(self, iterable):
		""" Make a MySQL SET from a list of values. Note that SET field type
		values my not contain spaces after the separating comma! """
		return ','.join(iterable)

	def insert(self, table_name, record_tuple):
		""" Dump a single record as INSERT <table> (<fields>) VALUES(...)
		while ignoring duplicates. MySQL syntax is 'INSERT IGNORE' """
		fields = filter_record(record_tuple)
		write('INSERT IGNORE', table_name,
			'(', ', '.join(fields.keys()), ')',
			'VALUES(', ', '.join(fields.values()), ')'
		)


class Postgres(object):
	def __init__(self, db_name = None):
		if db_name:
			write('\\c', db_name)

	def list(self, iterable):
		""" Make a PostgreSQL array from a list of values. Note: values are all
		ENUM types and don't require quoting. """
		return '{' + ','.join(iterable) + '}'

	def insert(self, table_name, record_tuple):
		""" Dump a single record as INSERT <table> (<fields>) VALUES(...)
		while ignoring duplicates. PostgerSQL syntax is 'INSERT ... ON CONFLICT
		DO NOTHING' to skip duplicates """
		fields = filter_record(record_tuple)
		write('INSERT INTO', table_name,
			'(', ', '.join(fields.keys()), ')',
			'VALUES(', ', '.join(fields.values()), ')',
			'ON CONFLICT DO NOTHING'
		)

if __name__ == '__main__':
    import sys
    sys.exit(0)
