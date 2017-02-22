#include <iostream>
#include <stdio.h>
#include <algorithm>

#include "Reader.h"

Vocabulary V;
unsigned int window_left = 2, window_right = 0;
std::string vocab_filename = "";
std::string unkown = "<UNK>", sos = "<S>", eos = "</S>";
//bool vocab_is_const = false;
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
