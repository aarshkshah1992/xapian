/* perftest_matchdecider.cc: performance tests for match decider
 *
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "perftest/perftest_matchdecider.h"

#include <xapian.h>

#include "backendmanager.h"
#include "perftest.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

using namespace std;

static Xapian::Database
builddb_valuestest1()
{
    std::string dbname("valuestest1");
    logger.testcase_begin("valuestest1");
    unsigned int runsize = 1000000;

    bool need_rebuild = false;
    try {
	Xapian::Database db = backendmanager->get_writable_database_as_database(dbname);
	if (db.get_doccount() != runsize)
	    need_rebuild = true;
    } catch (Xapian::DatabaseOpeningError & e) {
	need_rebuild = true;
    }

    if (need_rebuild) {
	// Rebuild the database.

	std::map<std::string, std::string> params;
	params["runsize"] = om_tostring(runsize);
	logger.indexing_begin(dbname, params);
	Xapian::WritableDatabase dbw = backendmanager->get_writable_database(dbname, "");
	unsigned int i;
	for (i = 0; i < runsize; ++i) {
	    unsigned int v = i % 100;
	    Xapian::Document doc;
	    doc.set_data("test document " + om_tostring(i));
	    doc.add_term("foo");
	    string vs = om_tostring(v);
	    if (vs.size() == 1) vs = "0" + vs;
	    doc.add_value(0, vs);
	    doc.add_term("F" + vs);
	    doc.add_term("Q" + om_tostring(i));
	    for (int j = 0; j != 100; ++j)
		doc.add_term("J" + om_tostring(j));
	    dbw.replace_document(i + 10, doc);
	    logger.indexing_add();
	}
	dbw.commit();
	logger.indexing_end();
	logger.testcase_end();
    }

    return backendmanager->get_writable_database_as_database(dbname);
}

// Test the performance of a ValueSetMatchDecider, compared to a Value range operator.
DEFINE_TESTCASE(valuesetmatchdecider1, writable && !remote && !inmemory) {
    Xapian::Database db = builddb_valuestest1();

    logger.testcase_begin("valuesetmatchdecider1");
    Xapian::Enquire enquire(db);
    Xapian::doccount runsize = db.get_doccount();

    Xapian::Query query("foo");
    logger.searching_start("No match decider");
    logger.search_start();
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 10, 0, NULL, NULL, NULL);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    TEST(mset.get_matches_upper_bound() <= runsize);

    logger.search_start();
    mset = enquire.get_mset(0, 10, 0, NULL, NULL, NULL);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    logger.searching_end();

    Xapian::ValueSetMatchDecider md(0, true);

    for (unsigned int i = 0; i < 100; ++i) {
	string vs = om_tostring(i);
	if (vs.size() == 1) vs = "0" + vs;
	md.add_value(vs);

	logger.searching_start("Match decider accepting " + om_tostring(i + 1) + "%");
	logger.search_start();
	enquire.set_query(query);
	mset = enquire.get_mset(0, 10, 0, NULL, &md, NULL);
	logger.search_end(query, mset);
	TEST_EQUAL(mset.size(), 10);
	TEST_REL(mset.get_matches_lower_bound(),<=,runsize * (i + 1) / 100);
	logger.searching_end();

	Xapian::Query query2(Xapian::Query::OP_FILTER, query,
			     Xapian::Query(Xapian::Query::OP_VALUE_LE, 0, vs));
	logger.searching_start("Value range LE accepting " + om_tostring(i + 1) + "%");
	Xapian::MSet mset2;
	logger.search_start();
	enquire.set_query(query2);
	mset2 = enquire.get_mset(0, 10, 0, NULL, NULL, NULL);
	logger.search_end(query2, mset2);
	TEST_EQUAL(mset2.size(), 10);
	TEST_REL(mset2.get_matches_lower_bound(),<=,runsize * (i + 1) / 100);
	test_mset_order_equal(mset, mset2);
	logger.searching_end();
    }

    logger.testcase_end();
    return true;
}

// Test the performance of an AllDocsIterator.
DEFINE_TESTCASE(alldocsiter1, writable && !remote && !inmemory) {
    Xapian::Database db = builddb_valuestest1();

    logger.testcase_begin("alldocsiter1");

    logger.searching_start("AllDocsPostingIterator, full iteration");
    logger.search_start();
    Xapian::PostingIterator begin(db.postlist_begin(""));
    Xapian::PostingIterator end(db.postlist_end(""));
    while (begin != end) {
	++begin;
    }
    logger.search_end(Xapian::Query(), Xapian::MSet());

    logger.searching_end();

    logger.testcase_end();
    return true;
}
