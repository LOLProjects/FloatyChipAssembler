#include <iostream>
#include <fstream>

#include "preprocessor.hpp"
#include "parser.hpp"
#include "assembler.hpp"

int main(int argc, char *argv[])
{
    try
    {
        std::string infile = "input.asm";
        std::string outfile = "output.bin";

        auto arguments = gsl::make_span(argv + 1, argc - 1);
        if (arguments.size() < 1)
        {
            std::cerr << "No input file" << std::endl;
            return -16;
        }

        if (arguments[0] == std::string("-h") || arguments[0] == std::string("--help"))
        {
            std::cout << "FloatyChip Assembler 0.0.1\n";
            std::cout << "Usage : FloatyChipAsm <input_file> <output_file>\n";
            return 0;
        }

        infile = arguments[0];
        if (arguments.size() >= 2)
        {
            outfile = arguments[1];
        }

        std::ifstream instream(infile);
        std::string instring;

        if (!instream.is_open()) {
            std::cerr << "Could not open input file: " << argv[1] << std::endl;
            return -16;
        }
        instream.unsetf(std::ios::skipws);
        instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
                               std::istreambuf_iterator<char>());

        floaty::pre_preprocess(instring);
        std::string str = floaty::preprocess(instring, infile);

        auto instructions = floaty::parse(str, infile);
        auto data = floaty::assemble(instructions);

        std::ofstream outstream(outfile, std::ios::trunc | std::ios::binary);
        outstream.write((const char*)data.data(), data.size());

        std::cout << "Compilation successful to file " << outfile << "\n";
    }
    catch (const floaty::pp_error& e)
    {
        std::cerr << "Error during preprocessing : " + std::string(e.what()) << std::endl;
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal exception : " + std::string(e.what()) << std::endl;
        return -4;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return -8;
    }

    return 0;
}
