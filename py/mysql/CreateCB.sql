-- MySQL
--
-- Version simplifiée du script de création des bases de données CC/CB. Le but
-- de cet exercice est de montrer à quel point il est possible de simplifier
-- le traitement et l'importation de données externes.
--
-- Le traitement et l'importation des données externes (fichier movies.txt)
-- sont confiés à un script en Python. L'injection des données se fait par
-- connexion SSH à travers l'exécution du client (p.ex. mysql) sur la machine
-- distante. Le script fournit un traitement d'erreur minimum.
--
-- Les spécificités de MySQL utilisées dans ce script sont:
--
-- * les types de champs SET et ENUM.
--
-- Le premier transforme une énumération en un masque de bits, tandis que le
-- second n'autorise qu'une seule valeur pour le champ considéré.
--
-- Les tables de certification, de genres et de statut sont ainsi évitées en
-- raison du petit nombre de valeurs qu'elles contiennent. De plus, la table
-- des certifications n'étant qu'un rappel des spécifications existantes (elles
-- sont disponibles en de nombreux endroit sur l'internet), c'est à la couche
-- applicative que revient la tâche de présentation des méta-données (code,
-- résumé, description, etc.). En cas de forte charge l'application peut aussi
-- mémoriser ces valeurs dans un cache, évitant ainsi de charger inutilement
-- le disque ou le système de stockage physique (disque).
--
-- Sous PostgreSQL, les type ENUM peut être facilement adapté sous la forme de
-- contraintes de type CHECK ou, mieux, d'un type utilisateur. Les champs SET,
-- quant à eux, seront transformés en tableaux (p.ex. "text[]"). Ce type de
-- conception n'est toutefois pas le plus adapté.
--
-- Voir https://www.postgresql.org/docs/8.4/arrays.html

-- TESTED:
-- * MySQL

-- Note: Conformité ANSI SQL:
-- s/varchar2/varchar, s/number/numeric

-- Note: Oracle ne semble pas se conformer à ANSI SQL quant à la définition de
-- VARCHAR, dont l'utilisation est dépréciée *soupirs*.
-- https://docs.oracle.com/cd/E11882_01/timesten.112/e21642/types.htm#TTSQL135

-- Note: MERISE - les noms de tables sont les noms, au pluriel, représentatifs
-- des objets.

USE movies;

-- Destruction des tables de liaison
DROP TABLE IF EXISTS directors;
DROP TABLE IF EXISTS characters;

-- Destruction des tables primaires
DROP TABLE IF EXISTS people;
DROP TABLE IF EXISTS movies;

-- La syntaxe DROP ... IF EXISTS est prise en charge sur les plateformes:
-- * SQL Server
-- * MySQL
-- * PostgreSQL
-- (pas Oracle, bien entendu...)

-- Note: "People" est le pluriel de "Person"
CREATE TABLE people (
  id        INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  full_name VARCHAR(31) NOT NULL CHECK (LENGTH(full_name) > 0)
);

CREATE TABLE movies (
  id             INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  title          VARCHAR(63) NOT NULL CHECK (LENGTH(title) > 0),
  original_title VARCHAR(63),
  tag_line       VARCHAR(79),
  status         ENUM('Released', 'Planned', 'In Production', 'Post Production',
                      'Canceled', 'Rumored'),
  genre          SET('Thriller', 'Family', 'TV Movie', 'Western', 'Science Fiction',
                     'Drama', 'Action', 'Crime', 'Romance', 'Animation', 'Documentary',
                     'History', 'War', 'Fantasy', 'Music', 'Comedy', 'Horror',
                     'Mystery', 'Adventure'),
  budget         DECIMAL(10, 2),
  release_date   DATE,
  vote_average   DECIMAL(3, 1),
  vote_count     INT UNSIGNED,
-- https://en.wikipedia.org/wiki/Motion_Picture_Association_of_America_film_rating_system
-- https://filmratings.com/Content/Downloads/mpaa_ratings-poster-qr.pdf
-- https://www.uecmovies.com/movies/ratings
-- https://filmratings.com/RatingsGuide
  certification  ENUM ('G', 'PG', 'PG-13', 'R', 'NC-17', 'X', 'NR', 'UR'),
  runtime        DECIMAL(3),	-- minutes
  poster_path    TEXT			-- URL uniquement
);

CREATE TABLE directors (
  movie_id    INT UNSIGNED NOT NULL,
  director_id INT UNSIGNED NOT NULL,
  FOREIGN KEY (movie_id) REFERENCES movies(id),
  FOREIGN KEY (director_id) REFERENCES people(id),
  UNIQUE INDEX (movie_id, director_id)
);

CREATE TABLE characters (
  movie_id INT UNSIGNED NOT NULL,
  actor_id INT UNSIGNED NOT NULL,
  -- ~ character_name VARCHAR(31) NOT NULL CHECK (LENGTH(character_name) > 0),
  character_name VARCHAR(31),
  FOREIGN KEY (movie_id) REFERENCES movies(id),
  FOREIGN KEY (actor_id) REFERENCES people(id),
  UNIQUE INDEX (movie_id, actor_id)
);
