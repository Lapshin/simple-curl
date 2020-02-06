#include "UrlParser.h"
#include "gtest/gtest.h"


using namespace std;

TEST(UrlParser, constructor)
{
    try
    {
        UrlParser up = UrlParser(string("http://go.mail.ru:8080/search?frm=tw&fr=main&q=linux"));
        EXPECT_EQ(up.getPort(), 8080);
        EXPECT_EQ(up.getPath(), "/search?frm=tw&fr=main&q=linux");
        EXPECT_EQ(up.getHost(), "go.mail.ru");
    }
    catch (...)
    {
        EXPECT_EQ(false, true);
    }

    try
    {
        UrlParser up = UrlParser(string("http://go.mail.ru:8080"));
        EXPECT_EQ(up.getPort(), 8080);
        EXPECT_EQ(up.getPath(), "/");
        EXPECT_EQ(up.getHost(), "go.mail.ru");
    }
    catch (std::runtime_error& e)
   {
       cerr << "Error while establish connection: " << endl;
       cerr << "\t" << e.what() << endl;
   }

    try
    {
        UrlParser up = UrlParser(string("http://go.mail.ru"));
        EXPECT_EQ(up.getPort(), 80);
        EXPECT_EQ(up.getPath(), "/");
        EXPECT_EQ(up.getHost(), "go.mail.ru");
    }
    catch (std::runtime_error& e)
   {
       cerr << "Error while establish connection: " << endl;
       cerr << "\t" << e.what() << endl;
   }

    try
    {
        UrlParser up = UrlParser(string("go.mail.ru:8080"));
        EXPECT_EQ(up.getPort(), 8080);
        EXPECT_EQ(up.getPath(), "/");
        EXPECT_EQ(up.getHost(), "go.mail.ru");
    }
    catch (std::runtime_error& e)
   {
       cerr << "Error while establish connection: " << endl;
       cerr << "\t" << e.what() << endl;
   }

    try
    {
        UrlParser up = UrlParser(string("go.mail.ru:8080/search?frm=tw&fr=main&q=linux"));
        EXPECT_EQ(up.getPort(), 8080);
        EXPECT_EQ(up.getPath(), "/search?frm=tw&fr=main&q=linux");
        EXPECT_EQ(up.getHost(), "go.mail.ru");
    }
    catch (...)
    {
        EXPECT_EQ(false, true);
    }

}

