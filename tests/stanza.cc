#include "stanza.h"
#include "gtest/gtest.h"
#include <iostream>
#include "rapidxml_print.hpp"

using namespace Metre;

class MessageTest : public ::testing::Test {
public:
    std::unique_ptr<Message> msg;
    rapidxml::xml_document<> doc;
    std::string msg_xml = "<message xmlns='jabber:server' from='foo@example.org/lmas' to='bar@example.net/laks' type='chat' id='1234'><body>This is the body &amp; stuff</body></message>";

    void SetUp() override {
        doc.parse<rapidxml::parse_fastest|rapidxml::parse_parse_one>(const_cast<char *>(msg_xml.c_str()));
        doc.fixup<rapidxml::parse_default>(doc.first_node(), false);
        msg = std::make_unique<Message>(doc.first_node());
    }

    static std::string print(Stanza & s) {
        rapidxml::xml_document<> tmp_doc;
        std::string tmp_buffer;
        s.render(tmp_doc);
        rapidxml::print(std::back_inserter(tmp_buffer), *(tmp_doc.first_node()), rapidxml::print_no_indenting);
        return tmp_buffer;
    }
};

TEST_F(MessageTest, IdMatches) {
    ASSERT_EQ(msg->id(), std::string("1234"));
}

TEST_F(MessageTest, MessageType) {
    ASSERT_EQ(msg->type(), Message::Type::CHAT);
}

TEST_F(MessageTest, MessageFrom) {
    ASSERT_EQ(msg->from().full(), "foo@example.org/lmas");
}

TEST_F(MessageTest, MessageTo) {
    ASSERT_EQ(msg->to().full(), "bar@example.net/laks");
}

TEST_F(MessageTest, MessageBodyConst) {
    const auto * msgc = msg.get();
    auto body = msgc->node()->first_node("body");
    std::string body_str{body->value(), body->value_size()};
    ASSERT_EQ(body_str, std::string("This is the body &amp; stuff"));
    msg->update();
    auto body1 = msg->node()->first_node("body");
    std::string body_str1{body1->value(), body1->value_size()};
    ASSERT_EQ(body_str1, std::string("This is the body & stuff"));
}

TEST_F(MessageTest, MessageBodyNonConst) {
    auto body = msg->node()->first_node("body");
    std::string body_str{body->value(), body->value_size()};
    ASSERT_EQ(body_str, std::string("This is the body & stuff"));
    auto body1 = msg->node()->first_node("body");
    std::string body_str1{body1->value(), body1->value_size()};
    ASSERT_EQ(body_str1, std::string("This is the body & stuff"));
}

TEST_F(MessageTest, MessageBodyUpdate) {
    msg->update();
    auto body = msg->node()->first_node("body");
    std::string body_str{body->value(), body->value_size()};
    ASSERT_EQ(body_str, std::string("This is the body & stuff"));
}

TEST_F(MessageTest, MessageBodyFreeze) {
    msg->freeze();
    auto body = msg->node()->first_node("body");
    std::string body_str{body->value(), body->value_size()};
    ASSERT_EQ(body_str, std::string("This is the body & stuff"));
}

TEST_F(MessageTest, MessageReplaceBody) {
    auto body = msg->node()->first_node("body");
    std::string replacement("Replacement body");
    body->value(replacement.data(), replacement.length());
    msg->update();
    auto body2 = msg->node()->first_node("body");
    ASSERT_TRUE(body2);
    std::string body_str{body2->value(), body2->value_size()};
    ASSERT_EQ(body_str, std::string("Replacement body"));
}

TEST_F(MessageTest, MessageReplaceBodyDynamicDouble) {
    Stanza * stanza = msg.get();
    {
        auto msg2 = dynamic_cast<Message *>(stanza);
        auto body = msg2->node()->first_node("body");
        std::string replacement("Replacement body");
        body->value(replacement.data(), replacement.length());
        msg2->update();
        auto body2 = msg2->node()->first_node("body");
        ASSERT_TRUE(body2);
        std::string body_str{body2->value(), body2->value_size()};
        ASSERT_EQ(body_str, std::string("Replacement body"));
    }
    {
        auto msg3 = dynamic_cast<Message *>(stanza);
        auto body3 = msg3->node()->first_node("body");
        std::string body_str_orig{body3->value(), body3->value_size()};
        ASSERT_EQ(body_str_orig, std::string("Replacement body"));
        std::string replacement("New replacement body");
        body3->value(replacement.data(), replacement.length());
        msg3->update();
        auto body4 = msg3->node()->first_node("body");
        ASSERT_TRUE(body4);
        std::string body_str{body4->value(), body4->value_size()};
        ASSERT_EQ(body_str, std::string("New replacement body"));
    }
}

TEST_F(MessageTest, MessageReplaceBodyDouble) {
    {
        auto body = msg->node()->first_node("body");
        std::string replacement("Replacement body");
        body->value(replacement.data(), replacement.length());
        msg->update();
        auto body2 = msg->node()->first_node("body");
        ASSERT_TRUE(body2);
        std::string body_str{body2->value(), body2->value_size()};
        ASSERT_EQ(body_str, std::string("Replacement body"));
    }
    {
        auto body3 = msg->node()->first_node("body");
        std::string body_str_orig{body3->value(), body3->value_size()};
        ASSERT_EQ(body_str_orig, std::string("Replacement body"));
        std::string replacement("New replacement body");
        body3->value(replacement.data(), replacement.length());
        msg->update();
        auto body4 = msg->node()->first_node("body");
        ASSERT_TRUE(body4);
        std::string body_str{body4->value(), body4->value_size()};
        ASSERT_EQ(body_str, std::string("New replacement body"));
    }
}

TEST_F(MessageTest, MessageReplaceBodyDoubleQuotes) {
    {
        auto body = msg->node()->first_node("body");
        std::string replacement("Replacement 'body'");
        body->value(replacement.data(), replacement.length());
        msg->update();
        auto body2 = msg->node()->first_node("body");
        ASSERT_TRUE(body2);
        std::string body_str{body2->value(), body2->value_size()};
        ASSERT_EQ(body_str, std::string("Replacement 'body'"));
    }
    {
        msg->update();
        auto body2 = msg->node()->first_node("body");
        ASSERT_TRUE(body2);
        std::string body_str{body2->value(), body2->value_size()};
        ASSERT_EQ(body_str, std::string("Replacement 'body'"));
    }
    {
        auto body3 = msg->node()->first_node("body");
        std::string body_str_orig{body3->value(), body3->value_size()};
        ASSERT_EQ(body_str_orig, std::string("Replacement 'body'"));
        std::string replacement("New replacement & '\"body'");
        body3->value(replacement.data(), replacement.length());
        msg->update();
        auto body4 = msg->node()->first_node("body");
        ASSERT_TRUE(body4);
        std::string body_str{body4->value(), body4->value_size()};
        ASSERT_EQ(body_str, replacement);
    }
    std::string tmp_buffer = print(*msg);;
    std::string expected = R"(<message to="bar@example.net/laks" from="foo@example.org/lmas" type="chat" id="1234"><body>New replacement &amp; &apos;&quot;body&apos;</body></message>)";
    ASSERT_EQ(tmp_buffer, expected);
}

TEST_F(MessageTest, ChangeType) {
    ASSERT_EQ(msg->type(), Message::Type::CHAT);
    ASSERT_EQ(msg->type_str(), "chat");
    msg->type(Message::Type::NORMAL);
    ASSERT_EQ(msg->type(), Message::Type::NORMAL);
    ASSERT_FALSE(msg->type_str().has_value());
    std::string tmp_buffer = print(*msg);
    std::string expected = R"(<message to="bar@example.net/laks" from="foo@example.org/lmas" id="1234"><body>This is the body &amp; stuff</body></message>)";
    ASSERT_EQ(tmp_buffer, expected);
}

TEST_F(MessageTest, Receipt) {
    auto receipt = msg->create_response();
    receipt->update();
    ASSERT_STREQ(receipt->node()->name(), "message");
    rapidxml::xml_document<> doc;
    auto tmp = doc.allocate_node(rapidxml::node_element, "tmp");
    auto receipt_el = doc.allocate_node(rapidxml::node_element, "receipt");
    receipt_el->append_attribute(doc.allocate_attribute("xmlns", "urn:xmpp:receipts"));
    tmp->append_node(receipt_el);
    receipt->payload(tmp);
    receipt->update();
    ASSERT_NE(receipt->node()->first_node(), nullptr);
    receipt->update();
    std::string tmp_buffer = print(*receipt);
    std::string expected = R"(<message to="foo@example.org/lmas" from="bar@example.net/laks" type="chat" id="1234"><receipt xmlns="urn:xmpp:receipts"/></message>)";
    ASSERT_EQ(tmp_buffer, expected);
}

TEST_F(MessageTest, Create)
{
    auto s = std::make_unique<Stanza>(Message::name, Jid{"from@example.org"}, Jid{"to@example.org"}, "chat", std::optional<std::string>());
    auto doc = s->node()->document();
    s->append_node(doc->allocate_node(rapidxml::node_element, "hello"));

    std::string tmp_buffer = print(*s);
    std::string expected = R"(<message to="to@example.org" from="from@example.org" type="chat"><hello/></message>)";
    ASSERT_EQ(tmp_buffer, expected);
}

TEST_F(MessageTest, Create2)
{
    auto s = std::make_unique<Stanza>(Message::name, Jid{"from@example.org"}, Jid{"to@example.org"}, "chat", "fish");
    s->append_node(s->allocate_element("hello", "urn:xmpp:hello"));

    {
        std::string tmp_buffer = print(*s);
        std::string expected = R"(<message to="to@example.org" from="from@example.org" type="chat" id="fish"><hello xmlns="urn:xmpp:hello"/></message>)";
        ASSERT_EQ(tmp_buffer, expected);
    }

    auto el = s->find_node("hello", "urn:xmpp:hello");

    s->remove_node(el);
    {
        std::string tmp_buffer = print(*s);
        std::string expected = R"(<message to="to@example.org" from="from@example.org" type="chat" id="fish"/>)";
        ASSERT_EQ(tmp_buffer, expected);
    }
}

TEST_F(MessageTest, Create3)
{
    auto s = std::make_unique<Stanza>(Message::name, Jid{"from@example.org"}, Jid{"to@example.org"}, "chat", "fish");
    auto el = s->allocate_element("hello", "urn:xmpp:hello");
    el->append_node(s->allocate_element("furry"));
    s->append_node(el);

    {
        std::string tmp_buffer = print(*s);
        std::string expected = R"(<message to="to@example.org" from="from@example.org" type="chat" id="fish"><hello xmlns="urn:xmpp:hello"><furry/></hello></message>)";
        ASSERT_EQ(tmp_buffer, expected);
    }

    auto el2 = s->find_node("hello", "urn:xmpp:hello");

    s->remove_node(el2);
    {
        std::string tmp_buffer = print(*s);
        std::string expected = R"(<message to="to@example.org" from="from@example.org" type="chat" id="fish"/>)";
        ASSERT_EQ(tmp_buffer, expected);
    }
}

class IqTest : public ::testing::Test {
public:
    std::unique_ptr<Iq> iq;
    rapidxml::xml_document<> doc;
    std::string iq_xml = "<iq xmlns='jabber:server' from='foo@example.org/lmas' to='bar@example.net/laks' type='get' id='1234'><query xmlns='urn:xmpp:ping'/></iq>";

    void SetUp() override {
        doc.parse<rapidxml::parse_fastest|rapidxml::parse_parse_one>(const_cast<char *>(iq_xml.c_str()));
        doc.fixup<rapidxml::parse_default>(doc.first_node(), false);
        iq = std::make_unique<Iq>(doc.first_node());
    }
};

TEST_F(IqTest, Id) {
    ASSERT_EQ(iq->id(), "1234");
}

TEST_F(IqTest, Type) {
    ASSERT_EQ(iq->type(), Iq::Type::GET);
}

TEST_F(IqTest, From) {
    ASSERT_EQ(iq->from().full(), "foo@example.org/lmas");
}

TEST_F(IqTest, To) {
    ASSERT_EQ(iq->to().full(), "bar@example.net/laks");
}

TEST_F(IqTest, Name) {
    ASSERT_STREQ(iq->node()->first_node()->name(), "query");
}

TEST_F(IqTest, Namespace) {
    ASSERT_STREQ(iq->node()->first_node()->xmlns(), "urn:xmpp:ping");
}

#if 0
class IqGenTest : public Test {
public:
    IqGenTest() : Test("Iq Gen") {}

    bool run() {
        Iq iq(std::string("foo@example.org/lmas"), std::string("bar@example.net/laks"), Iq::GET, std::string("1234"));
        iq.payload("<ping xmlns='urn:xmpp:ping'/>");
        rapidxml::xml_document<> doc;
        iq.render(doc);
        std::string tmp;
        rapidxml::print(std::back_inserter(tmp), doc, rapidxml::print_no_indenting);
        assert::equal(tmp,
                      "<iq to=\"bar@example.net/laks\" from=\"foo@example.org/lmas\" type=\"get\" id=\"1234\"><ping xmlns='urn:xmpp:ping'/></iq>",
                      "iq/render");
        return true;
    }
};

namespace {
    MessageTest messagetest;
    IqTest iqtest;
    IqGenTest iqgentest;
}
#endif