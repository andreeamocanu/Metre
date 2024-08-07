/***

Copyright 2013-2016 Dave Cridland
Copyright 2014-2016 Surevine Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

***/

#include "stanza.h"
#include "xmlstream.h"
#include "rapidxml_print.hpp"
#include "log.h"

using namespace Metre;

Stanza::Stanza(const char *name, rapidxml::xml_node<> *node) : m_name(name), m_node(node) {
    auto to = node->first_attribute("to");
    if (to) m_to = Jid(to->value());
    auto from = node->first_attribute("from");
    if (from) m_from = Jid(from->value());
    auto typestr = node->first_attribute("type");
    if (typestr) m_type_str = typestr->value();
    auto id = node->first_attribute("id");
    if (id) m_id = id->value();
    m_payload = node->contents();
    m_payload_l = node->contents_size();
}

Stanza::Stanza(const char *name) : m_name(name) {
}

Stanza::Stanza(const char *name, Jid const &from, Jid const &to, std::string const &type_str,
               std::optional<std::string> const &id)
        : m_name(name), m_from(from), m_to(to), m_type_str(type_str),
          m_id(id) {
}

void Stanza::freeze() {
    if (!m_payload_str.empty()) return;
    if (!m_payload) {
        m_node = nullptr;
        return;
    }
    m_payload_str.assign(m_payload, m_payload_l);
    m_payload = m_payload_str.data();
    m_node = nullptr;
}

void Stanza::payload(rapidxml::xml_node<> *node) {
    std::string tmp_buffer;
    for(rapidxml::xml_node<> *child = node->first_node(); child; child = child->next_sibling()) {
        std::string node_name{child->name(), child->name_size()};
        if (child->value_size()) {
            std::string node_value{child->value(), child->value_size()};
        }
        rapidxml::print(std::back_inserter(tmp_buffer), *child, rapidxml::print_no_indenting);
    }
    std::swap(m_payload_str, tmp_buffer);
    m_payload = m_payload_str.data();
    m_payload_l = m_payload_str.length();
    m_node = nullptr;
}

void Stanza::append_node(rapidxml::xml_node<>* new_node) {
    auto orig = node_internal();
    orig->append_node(new_node);
    payload(orig);
}

void Stanza::remove_node(rapidxml::xml_node<>* new_node) {
    auto orig = node_internal();
    orig->remove_node(new_node);
    payload(orig);
}

rapidxml::xml_node<>* Stanza::allocate_element(std::string const& element) {
    auto tmp = node_internal()->document()->allocate_string(element.data(), element.size());
    return node_internal()->document()->allocate_node(rapidxml::node_element, tmp, 0, element.size());
}

rapidxml::xml_node<>* Stanza::allocate_element(std::string const& element, std::optional<std::string> const& xmlns) {
    auto n = allocate_element(element);
    if (xmlns.has_value()) {
        auto tmp = node_internal()->document()->allocate_string(xmlns.value().data(), xmlns.value().size());
        n->append_attribute(node_internal()->document()->allocate_attribute("xmlns", tmp, 5, xmlns.value().size()));
    }
    return n;
}

rapidxml::xml_node<>* Stanza::find_node(std::string const& element, std::optional<std::string> const& xmlns) {
    return node_internal()->first_node(
        element.data(),
        xmlns.has_value() ? xmlns.value().data() : nullptr,
        element.size(),
        xmlns.has_value() ? xmlns.value().size() : 0
        );
}

rapidxml::xml_node<>* Stanza::find_node(std::string const & element) {
    return find_node(element, std::optional<std::string>{});
}

void Stanza::render(rapidxml::xml_document<> &d) {
    auto hdr = d.allocate_node(rapidxml::node_element, m_name);
    if (m_to) {
        auto att = d.allocate_attribute("to", m_to->full().c_str());
        hdr->append_attribute(att);
    }
    if (m_from) {
        auto att = d.allocate_attribute("from", m_from->full().c_str());
        hdr->append_attribute(att);
    }
    if (m_type_str) {
        auto att = d.allocate_attribute("type", m_type_str->c_str());
        hdr->append_attribute(att);
    }
    if (m_id) {
        auto att = d.allocate_attribute("id", m_id->c_str());
        hdr->append_attribute(att);
    }
    if (m_payload && m_payload_l) {
        auto lit = d.allocate_node(rapidxml::node_literal);
        lit->value(m_payload, m_payload_l);
        hdr->append_node(lit);
    }
    d.append_node(hdr);
}

rapidxml::xml_node<> *Stanza::node_internal() {
    if (m_node) return m_node;
    rapidxml::xml_document<> tmp_doc;
    std::string tmp{m_payload, m_payload_l}; // Copy the buffer.
    m_payload = tmp.data();
    m_payload_l = tmp.length(); // Reset pointers to buffer.
    render(tmp_doc);
    std::string tmp_buffer;
    rapidxml::print(std::back_inserter(tmp_buffer), *(tmp_doc.first_node()), rapidxml::print_no_indenting);
    m_doc.reset(new rapidxml::xml_document<>);
    m_doc->parse<rapidxml::parse_fastest>(const_cast<char *>(tmp_buffer.c_str()));
    std::swap(m_node_str, tmp_buffer);
    m_node = m_doc->first_node();
    m_payload_str.assign(m_node->contents(), m_node->contents_size());
    m_payload = m_payload_str.data();
    m_payload_l = m_payload_str.size();
    m_doc->fixup<rapidxml::parse_default|rapidxml::parse_no_data_nodes>(m_doc->first_node(), true);
    return m_node;
}

std::unique_ptr<Stanza> Stanza::create_bounce(base::stanza_exception const &ex) const {
    std::unique_ptr<Stanza> stanza{new Stanza(m_name)};
    stanza->m_from = m_to;
    stanza->m_to = m_from;
    stanza->m_id = m_id;
    stanza->m_type_str = "error";
    stanza->render_error(ex);
    if (m_payload && m_payload_l) {
        stanza->m_payload_str.append(m_payload, m_payload_l);
        stanza->m_payload = stanza->m_payload_str.c_str();
        stanza->m_payload_l = stanza->m_payload_str.length();
    }
    return stanza;
}

void Stanza::render_error(Metre::base::stanza_exception const &ex) {
    // Render the error
    rapidxml::xml_document<> d;
    auto error = d.allocate_node(rapidxml::node_element, "error");
    error->append_attribute(d.allocate_attribute("type", ex.error_type()));
    d.append_node(error);
    auto condition = d.allocate_node(rapidxml::node_element, ex.element_name());
    condition->append_attribute(d.allocate_attribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas"));
    error->append_node(condition);
    auto text = d.allocate_node(rapidxml::node_element, "text");
    text->append_attribute(d.allocate_attribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas"));
    text->value(ex.what());
    error->append_node(text);
    rapidxml::print(std::back_inserter(m_payload_str), d, rapidxml::print_no_indenting);
}

void Stanza::render_error(Stanza::Error e) {
    switch (e) {
        case remote_server_timeout:
            render_error(stanza_remote_server_timeout());
            return;
        case remote_server_not_found:
            render_error(stanza_remote_server_not_found());
            return;
        case service_unavailable:
            render_error(stanza_service_unavailable());
            return;
        case undefined_condition:
            render_error(stanza_undefined_condition());
            return;
        case policy_violation:
            render_error(stanza_policy_violation());
            return;
        default:
        METRE_LOG(Log::CRIT, "Unhandled stanza error type");
            render_error(stanza_undefined_condition());
    }
}

std::unique_ptr<Stanza> Stanza::create_bounce(Stanza::Error e) const {
    switch (e) {
        case remote_server_timeout:
            return create_bounce(stanza_remote_server_timeout());
        case remote_server_not_found:
            return create_bounce(stanza_remote_server_not_found());
        case service_unavailable:
            return create_bounce(stanza_service_unavailable());
        case undefined_condition:
            return create_bounce(stanza_undefined_condition());
        default:
        METRE_LOG(Log::CRIT, "Unhandled stanza error type");
            return create_bounce(stanza_undefined_condition());
    }
}

std::unique_ptr<Stanza> Stanza::create_forward() const {
    std::unique_ptr<Stanza> stanza{new Stanza(m_name)};
    stanza->m_from = m_from;
    stanza->m_to = m_to;
    stanza->m_id = m_id;
    stanza->m_type_str = m_type_str;
    if (m_payload && m_payload_l) {
        stanza->m_payload_str.append(m_payload, m_payload_l);
        stanza->m_payload = stanza->m_payload_str.c_str();
        stanza->m_payload_l = stanza->m_payload_str.length();
    }
    return stanza;
}

const char *Stanza::error_name(Stanza::Error err) {
    std::vector<const char *> names{
            "bad-request",
            "conflict",
            "feature-not-implemented",
            "forbidden",
            "gone",
            "internal-server-error",
            "item-not-found",
            "jid-malformed",
            "not-acceptable",
            "not-allowed",
            "not-authorized",
            "policy-violation",
            "recipient-unavailable",
            "redirect",
            "registration-required",
            "remote-server-not-found",
            "remote-server-timeout",
            "resource-constraint",
            "service-unavailable",
            "subscription-required",
            "undefined-condition",
            "unexpected-request"
    };
    return names.at(static_cast<std::vector<const char *>::size_type>(err));
}

Message::Message() : Stanza(Message::name) {
    m_type = set_type();
}

Message::Message(rapidxml::xml_node<> *node) : Stanza(Message::name, node) {
    m_type = set_type();
}

std::unique_ptr<Message> Message::create_response() const {
    auto stanza = std::make_unique<Message>();
    stanza->m_from = m_to;
    stanza->m_to = m_from;
    stanza->m_id = m_id;
    stanza->m_type_str = m_type_str;
    stanza->m_type = m_type;
    return stanza;
}



void Message::type(Message::Type t) {
    m_type = t;
    switch(m_type) {
        case NORMAL:
            type_str(std::optional<std::string>());
            break;
        case CHAT:
            type_str("chat");
            break;
        case HEADLINE:
            type_str("headline");
            break;
        case GROUPCHAT:
            type_str("groupchat");
            break;
        case STANZA_ERROR:
            type_str("error");
            break;
    }
}

Message::Type Message::set_type() const {
    if (!type_str()) return NORMAL;
    std::string const &t = *type_str();
    switch (t[0]) {
        case 'n':
            if (t == "normal") return NORMAL;
            break;
        case 'c':
            if (t == "chat") return CHAT;
            break;
        case 'h':
            if (t == "headline") return HEADLINE;
            break;
        case 'g':
            if (t == "groupchat") return GROUPCHAT;
            break;
        case 'e':
            if (t == "error") return STANZA_ERROR;
            break;
    }
    throw std::runtime_error("Unknown Message type");
}

Iq::Iq(Jid const &from, Jid const &to, Type t, std::optional<std::string> const &id) : Stanza(Iq::name, from, to,
                                                                                              Iq::type_toString(t), id), m_type(t) {}

Iq::Iq(rapidxml::xml_node<> *node) : Stanza(name, node) {
    m_type = set_type();
}

const char *Iq::type_toString(Type t) {
    switch (t) {
        case GET:
            return "get";
        case SET:
            return "set";
        case RESULT:
            return "result";
        case STANZA_ERROR:
            return "error";
    }
    return "error";
}

Iq::Type Iq::set_type() const {
    if (!type_str()) throw std::runtime_error("Missing type for Iq");
    std::string const &t = *type_str();
    switch (t[0]) {
        case 'g':
            if (t == "get") return GET;
            break;
        case 's':
            if (t == "set") return SET;
            break;
        case 'r':
            if (t == "result") return RESULT;
            break;
        case 'e':
            if (t == "error") return STANZA_ERROR;
            break;
    }
    throw std::runtime_error("Unknown IQ type");
}

rapidxml::xml_node<> const &Iq::query() const {
    return *node()->first_node();
}

const char *Iq::name = "iq";
const char *Message::name = "message";
const char *Presence::name = "presence";
const char *DB::Verify::name = "db:verify";
const char *DB::Result::name = "db:result";

/*
 * Dialback
 */

DB::DB(const char *name, Jid const &to, Jid const &from, std::string const &stream_id,
       std::optional<std::string> const &key)
        : Stanza(name) {
    m_to = to;
    m_from = from;
    m_id = stream_id;
    m_payload_str = *key;
    m_payload = m_payload_str.data();
    m_payload_l = m_payload_str.length();
}

DB::DB(const char *name, Jid const &to, Jid const &from, std::string const &stream_id, Type t) : Stanza(name) {
    m_to = to;
    m_from = from;
    m_id = stream_id;
    switch (t) {
        case VALID:
            m_type_str = "valid";
            break;
        case INVALID:
            m_type_str = "invalid";
            break;
        case STANZA_ERROR:
            m_type_str = "error";
            break;
    }
}

DB::DB(const char *name, Jid const &to, Jid const &from, std::string const &stream_id, Stanza::Error e) : Stanza(name) {
    m_to = to;
    m_from = from;
    m_id = stream_id;
    m_type_str = "error";
    render_error(e);
    m_payload = m_payload_str.data();
    m_payload_l = m_payload_str.size();
}