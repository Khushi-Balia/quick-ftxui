#ifndef QUICK_FTXUI_HPP
#define QUICK_FTXUI_HPP

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/function.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <iostream>
#include <string>

namespace client {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace quick_ftxui_ast {
///////////////////////////////////////////////////////////////////////////
//  The AST
///////////////////////////////////////////////////////////////////////////
struct nil {};
struct button;
struct expression;
// struct function;

typedef boost::variant<nil, boost::recursive_wrapper<button>,
                       boost::recursive_wrapper<expression>>
    node;

struct button {
    std::string placeholder;
    std::string func;
};

struct expression {
    button first;
    std::list<node> rest;
};

// print function for debugging
inline std::ostream &operator<<(std::ostream &out, nil) {
    out << "nil";
    return out;
}

// print function for debugging
inline std::ostream &operator<<(std::ostream &out, button b) {
    out << "Button placeholder: " << b.placeholder << "Button func: " << b.func;
    return out;
}

} // namespace quick_ftxui_ast
} // namespace client

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::button,
                          (std::string, placeholder)
                          (std::string, func)
)


BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::expression,
                          (client::quick_ftxui_ast::button, first)
                          (std::list<client::quick_ftxui_ast::node>, rest)
)

// clang-format on

namespace client {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


namespace quick_ftxui_parser {
using boost::phoenix::function;

///////////////////////////////////////////////////////////////////////////////
//  The error handler
///////////////////////////////////////////////////////////////////////////////
struct error_handler_ {
    template <typename, typename, typename> struct result {
        typedef void type;
    };

    template <typename Iterator>
    void operator()(qi::info const &what, Iterator err_pos,
                    Iterator last) const {
        std::cout << "Error! Expecting " << what // what failed?
                  << " here: \""
                  << std::string(err_pos, last) // iterators to error-pos, end
                  << "\"" << std::endl;
    }
};

function<error_handler_> const error_handler = error_handler_();

template <typename Iterator>
struct parser
    : qi::grammar<Iterator, quick_ftxui_ast::expression(), ascii::space_type> {
    parser() : parser::base_type(expression) {
        qi::char_type char_;
        qi::uint_type uint_;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;

        using qi::fail;
        using qi::on_error;

        quoted_string %= qi::lexeme['"' >> +(char_ - '"') >> '"'];

        button_comp %= qi::lit("Button") >> '{' >> quoted_string >> ',' >>
                       quoted_string >> '}';

        node = button_comp | expression;

        expression = '{' >> button_comp >> *node >> '}';

        // Debugging and error handling and reporting support.
        BOOST_SPIRIT_DEBUG_NODES((button_comp)(expression));

        // Error handling
        on_error<fail>(button_comp, error_handler(_4, _3, _2));
    }
    qi::rule<Iterator, quick_ftxui_ast::expression(), ascii::space_type>
        expression;
    qi::rule<Iterator, quick_ftxui_ast::node(), ascii::space_type> node;
    qi::rule<Iterator, quick_ftxui_ast::button(), ascii::space_type>
        button_comp;
    qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
};
} // namespace quick_ftxui_parser

} // namespace client

#endif // QUICK_FTXUI_HPP
