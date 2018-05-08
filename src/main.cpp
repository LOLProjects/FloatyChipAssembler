#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "preprocessor.hpp"
#include "parser.hpp"
#include "assembler.hpp"

int main(int argc, char *argv[])
{
    try
    {

        namespace po = boost::program_options;
        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "Prints the help message")
                ("input,i", po::value<std::string>(), "input file")
                ("output,o", po::value<std::string>(), "output file")
                ("format,f", po::value<std::string>(), "output format");

        po::positional_options_description p;
        p.add("input", 1);
        p.add("output", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(p).run(), vm);
        po::notify(vm);

        std::string infile = "input.asm";
        std::string outfile = "output.bin";
        std::string outformatstr = "raw";
        enum class Format
        {
            Raw
        } outformat = Format::Raw;

        if (vm.count("help") || vm.empty())
        {
            std::cout << "FloatyChip Assembler 0.0.1\n";
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("input"))
        {
            infile = vm["input"].as<std::string>();
        }

        if (vm.count("output"))
        {
            outfile = vm["output"].as<std::string>();
        }

        if (vm.count("format"))
        {
            outformatstr = vm["format"].as<std::string>();
        }

        if (outformatstr == "raw")
        {
            outformat = Format::Raw;
        }
        else
        {
            std::cerr << "Invalid output format : " << outformatstr << " (types : 'raw')";
            return -32;
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
