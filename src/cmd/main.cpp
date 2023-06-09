#include "core.hpp"
#include "parser.hpp"
#include "regex/format.hpp"
#include <fmt/core.h>
#include <fstream>
using namespace bee;

s32 main(s32 argc, const char *argv[])
{
    if (argc != 2)
    {
        fmt::print("Bee - Cmd interface\n");
        return 0;
    }

    std::fstream fstream{argv[1]};
    if (!fstream.is_open())
    {
        throw Error{"fstream error", fmt::format("cannot open source from: '{:s}'", argv[1])};
    }

    std::string src{std::istreambuf_iterator{fstream}, {}};
    Scanner scanner{src, bee_syntax_map()};
    Ast ast = Parser{scanner}.parse();
    if (!ast.exprs.empty())
    {
        Ast_Dump_Stream ast_dump_stream;
        ast.exprs.back()->ast_dump(ast_dump_stream, 0);
        fmt::print("{:s}\n", ast_dump_stream.str());
        ast.free();
    }
}

// s32 main(s32 argc, const char *argv[])
// {
//     switch (argc)
//     {
//     case 2: {
//         Regex regex = argv[1];
//         fmt::print("{:s}\n", regex::Format{regex.src, regex.head}.str());
//         return 0;
//     }

//     case 3: {
//         Regex regex = argv[1];
//         std::string_view expr = argv[2];
//         regex::Match match = regex.match(expr);

//         if (match.index != npos)
//             fmt::print("'{:s}' with '{:s}' matched '{:s}'", regex.src, expr, match.view());
//         else
//             fmt::print("'{:s}' with '{:s}' didn't matched !", regex.src, expr);
//         return 0;
//     }

//     default:
//         fmt::print("Bee - Cmd interface\n");
//         return 0;
//     }
// }
