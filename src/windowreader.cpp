#include <iostream>
#include <stdio.h>
#include <algorithm>

#include "Reader.h"

Vocabulary V;
unsigned int window_left = 2, window_right = 0;
std::string vocab_filename = "";
std::string unkown = "<UNK>", sos = "<S>", eos = "</S>";
unsigned int step = 1;
FILE* input = stdin;
std::string space = " ", place = "";
bool pan = false;
size_t every = 1;

int main(int argc, char* argv[])
{
	for (++argv; *argv != nullptr; ++argv)
	{
		std::string arg(*argv);
		if (arg == "-v" || arg == "--vocab")
			vocab_filename = *++argv;
        else if (arg == "-w" || arg == "--window")
            window_left = window_right = atoi(*++argv);
        else if (arg == "-l" || arg == "--left")
            window_left = atoi(*++argv);
        else if (arg == "-r" || arg == "--right")
            window_right = atoi(*++argv);
        else if (arg == "--sos")
            sos = *++argv;
        else if (arg == "--eos")
            eos = *++argv;
        else if (arg == "--pan")
            pan = true;
        else if (arg == "-u" || arg == "--unk")
            unkown = *++argv;
        else if (arg == "-i" || arg == "--input")
            input = fopen(*++argv, "rb");
        else if (arg == "-s" || arg == "--space")
            space = *++argv;
        else if (arg == "-e" || arg == "--every")
            every = std::max(atoi(*++argv), 1);
        else if (arg == "-p" || arg == "--pad")
            place = *++argv;
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << 
            "Window Reader\nauthor: Gabor Borbely, contact: borbely@math.bme.hu\n" <<
            std::endl << 
            "    <s>   This     is      a sample" << std::endl <<
            "   This     is      a sample   text" << std::endl <<
            "     is      a sample   text    for" << std::endl <<
            "      a sample   text    for    the" << std::endl <<
            " sample   text    for    the window" << std::endl <<
            "   text    for    the window reader" << std::endl <<
            "    for    the window reader      !" << std::endl <<
            "    the window reader      !   </s>" << std::endl <<
            std::endl << 
            "Reads text from stdin and breaks the text down to windows." << std::endl <<
            "Outputs one window per line. New sentence breaks a window." << std::endl <<
            "Token delimiters: space, tab (\\t) and vertical tab (\\v)." << std::endl <<
            "Sentence delimiters: newline, carriage return (\\r) and form feed (\\f)." << std::endl <<
            "\nOptional arguments:" << std::endl <<
            "-v\t--vocab\tvocabulary, default: \"" << vocab_filename << "\"" << std::endl <<
            "-w\t--window\twindow to both sides" << std::endl <<
            "\t\ta symmetric window of 2 includes 5 words" << std::endl <<
            "\t\tlike in the example above" << std::endl <<
            "-l\t--left\twindow to the left, default: " << window_left << std::endl <<
            "-r\t--right\twindow to the right, default: " << window_right << std::endl <<
            "-i\t--input\tinput file name, uses stdin as input if not given otherwise" << std::endl <<
            "-s\t--space\tspecial space symbol used in output, default: \"" << space << "\"" << std::endl <<
            "-p\t--pad\tpadding symbol used in panning, default: \"" << place << "\"" << std::endl <<
            "-e\t--every\tprints out not every line but every n-th line" << std::endl <<
            "\t\tdefault: n=" << every << std::endl <<
            "\t--pan\tflag indicating to pan the window, default: " << (pan ? "true" : "false") << std::endl <<
            "\t\tpan out looks like this:\n"<< std::endl <<
            "\t\t___ ___ the dog barks"<< std::endl <<
            "\t\t___ the dog barks   ."<< std::endl <<
            "\t\tthe dog barks   . ___"<< std::endl <<
            "\t\tdog barks   . ___ ___\n"<< std::endl <<
            "\t--unk\tunknown symbol, default: \"" << unkown << "\"" << std::endl <<
            "\t\tonly effective when using a vocabulary" << std::endl <<
            "\t--sos\tstart-of-sentence symbol, default: \"" << sos << "\"" << std::endl <<
            "\t--eos\tend-of-sentence symbol, default: \"" << eos << "\"" << std::endl <<
            std::endl << 
            "-h\t--help\tprint out this text and exit" << std::endl;
            
            return 0;
        }
        else
            std::cerr << "Unknown parameter: \"" << arg << "\"!" << std::endl;
	}
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	if (!vocab_filename.empty())
	{
        FILE* fin = fopen(vocab_filename.c_str(), "r");
        if (fin)
        {
            V = ReadVocab(fin);
            fclose(fin);
        }else
        {
            std::cerr << "Unable to open file \"" << vocab_filename << "\"!" << std::endl;
			return 1;
		}
        for (auto exception : { unkown, sos, eos })
        {
            if (V.find(exception) == V.end())
            {
                std::cerr << "Token \"" << exception << "\" not in vocabulary!" << std::endl;
                return 1;
            }
        }
        IndexReader reader(input, V, unkown, sos, eos);
        // TODO make sos and eos optional!
        WindowsReader<IndexReader> wr(&reader, window_left, window_right, space, place, V.at(sos), V.at(eos), pan);
        while (wr.IsGood())
        {
            wr.Print(stdout);
            for (size_t i = 0; i < every; ++i)
                if (wr.ReadItem() || wr.IsSentenceBoundary())
                    break;
        }
    }
    else{
        Reader reader(input, sos, eos);
        WindowsReader<Reader> wr(&reader, window_left, window_right, space, place, sos, eos, pan);
        while (wr.IsGood())
        {
            wr.Print(stdout);
            for (size_t i = 0; i < every; ++i)
                if (wr.ReadItem() || wr.IsSentenceBoundary())
                    break;
        }
    }
	return 0;
}
