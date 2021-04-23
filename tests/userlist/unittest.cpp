// unit_test.cpp
//
// Tests for the userlist and useritem classes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <clientlist.h>
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
TEST(ClientList, setUserName_getName_fixName) 
{ 
    CUserItem item;

    item.setUserName("MrSmith");
    ASSERT_EQ("MrSmith", item.getUserName());

    item.setUserName("Mr Smith");
    item.fixName();
    ASSERT_EQ("Mr_Smith", item.getUserName());

    item.setUserName("Mr 1\"and\'Smith,t");
    item.fixName();
    ASSERT_EQ("Mr_1_and_Smith_t", item.getUserName());
    
    // uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
    // ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserID_getUserID) 
{ 
    CUserItem item;

    item.setUserID(0);
    ASSERT_EQ(0, item.getUserID());

    item.setUserID(0xffffffff);
    ASSERT_EQ(0xffffffff, item.getUserID());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setPassword_getPassword) 
{ 
    CUserItem item;

    item.setPassword("Very secret this password is");
    ASSERT_EQ("Very secret this password is", item.getPassword());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setFullname_getFullname) 
{ 
    CUserItem item;

    item.setFullname("Mr Smith");
    ASSERT_EQ("Mr Smith", item.getFullname());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setNote_getNote) 
{ 
    CUserItem item;

    item.setNote("This is some text describing this user");
    ASSERT_EQ("This is some text describing this user", item.getNote());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRights_getUserRights) 
{ 
    CUserItem item;

    item.setUserRights(0xffffffffffffffff);
    ASSERT_EQ(0xffffffffffffffff, item.getUserRights());
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
