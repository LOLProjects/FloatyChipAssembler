/*
preprocessor.cpp

Copyright (c) 05 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "preprocessor.hpp"

#include <boost/wave.hpp>

#include <boost/wave/cpplexer/cpp_lex_token.hpp>    // token class
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp> // lexer class

#include <boost/algorithm/string/replace.hpp>

namespace floaty
{

std::string preprocess(std::string_view input, std::string_view filename)
{
    boost::wave::util::file_position_type current_position;
    try
    {
        //  This token type is one of the central types used throughout the library.
        //  It is a template parameter to some of the public classes and instances
        //  of this type are returned from the iterators.
        typedef boost::wave::cpplexer::lex_token<> token_type;

        //  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
        //  to use as the token source for the preprocessing engine. It is
        //  parametrized with the token type.
        typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;

        //  This is the resulting context type. The first template parameter should
        //  match the iterator type used during construction of the context
        //  instance (see below). It is the type of the underlying input stream.
        typedef boost::wave::context<std::string_view::iterator, lex_iterator_type>
                context_type;

        //  The preprocessor iterator shouldn't be constructed directly. It is
        //  generated through a wave::context<> object. This wave:context<> object
        //  is additionally used to initialize and define different parameters of
        //  the actual preprocessing (not done here).
        //
        //  The preprocessing of the input stream is done on the fly behind the
        //  scenes during iteration over the range of context_type::iterator_type
        //  instances.
        context_type ctx (input.begin(), input.end(), filename.data());
        boost::wave::language_support lang = ctx.get_language();
        //lang = boost::wave::enable_emit_line_directives(lang, false);
        ctx.set_language(lang);

        //  Get the preprocessor iterators and use them to generate the token
        //  sequence.
        context_type::iterator_type first = ctx.begin();
        context_type::iterator_type last = ctx.end();

        //  The input stream is preprocessed for you while iterating over the range
        //  [first, last). The dereferenced iterator returns tokens holding
        //  information about the preprocessed input stream, such as token type,
        //  token value, and position.

        std::string processed;

        while (first != last) {
            current_position = (*first).get_position();
            processed += (*first).get_value().c_str();
            ++first;
        }
        return processed;
    }
    catch (boost::wave::cpp_exception const& e) {
        // some preprocessing error
        pp_error_throw(std::string(e.file_name()) + "(" + std::to_string(e.line_no()) + "): "
                       + e.description());
    }
    catch (std::exception const& e) {
        // use last recognized token to retrieve the error position
        pp_error_throw(std::string(current_position.get_file().c_str()) + "(" +
                       std::to_string(current_position.get_line()) +
                       "): " + "exception caught: " + e.what());
    }
    catch (...) {
        pp_error_throw(std::string(current_position.get_file().c_str()) + "(" +
                       std::to_string(current_position.get_line()) +
                       "): " + "Unknown exception caught");
    }
}

void pre_preprocess(std::string &input)
{
    boost::replace_all(input, ";", "//");
}

}
