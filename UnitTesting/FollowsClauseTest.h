#pragma once

// Note 1
#include <cppunit/extensions/HelperMacros.h>

class FollowsClauseTest : public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( FollowsClauseTest );
	CPPUNIT_TEST( testIsFollows );
	CPPUNIT_TEST( testIsFollowsStar );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testIsFollows();

	void testIsFollowsStar();

};
    