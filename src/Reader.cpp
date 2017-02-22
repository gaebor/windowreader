#include <algorithm>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>

#include "Reader.h"

const char* Format<std::string>::str = "%s";

template<>
const char* Format<long long>::str = "%lld";

// due to portability issues
#ifdef _MSC_VER
template<>
const char* Format<size_t>::str = "%Iu";
#define ftello _ftelli64
#elif defined __GNUC__
template<>
const char* Format<size_t>::str = "%zu";
#endif // _MSC_VER

Vocabulary ReadVocab(const char* fname)
{
    Vocabulary V;
    std::string word;
    size_t j = 0;

    std::ifstream ifs(fname, std::ifstream::in);
    while (ifs)
    {
        ifs >> word;
        V[word] = j++;
        std::getline(ifs, word);
    }
    return V;
}

size_t IndexReader::GetIndex(const std::string& w) const
{
    auto where = _w2i->find(w);
    return where == _end ? _unk_index : where->second;
}

IndexReader::IndexReader(FILE* fin, const Vocabulary& v, std::string unk_token, std::string sos/*="<s>"*/, std::string eos/*="</s>"*/)
    : Reader(fin, sos, eos), _w2i(&v), _unk_index(v.at(unk_token))
{
    _end = v.end();
}

size_t IndexReader::GetItem() const
{
    return GetIndex(static_cast<const Reader*>(this)->GetItem());
}

Reader::Reader(FILE* fin, std::string s/*="<s>"*/, std::string e/*="</s>"*/)
    : state(EOS), fin(fin), pos(ftello(fin)), word(), sos(s), eos(e)
{
    if (fin == NULL)
        state = EOF_S;
}

bool Reader::ReadNext()
{
    static int ch;
    if(state == EOS_P)
    {
        state = EOS;
        actual = &eos;
        return true;
    }
    
    word.clear();
    actual = &word;
    pos += 1;

    for (ch = fgetc(fin);; ch = fgetc(fin), ++pos)
        switch (state)
    {
        case INW:
            switch (ch)
            {
            case ' ': case '\t': case '\r':
                state = OUTW;
                return true;
            case '\v': case '\n': case '\f': case EOF:
                state = EOS_P;
                return true;
            default:
                word.push_back(ch);
                break;
            }
            break;
        case OUTW:
            switch (ch)
            {
            case ' ': case '\t': case '\r':
                break;
            case '\v': case '\n': case '\f':
                state = EOS;
                actual = &eos;
                return true;
            case EOF:
                state = EOF_S;
                return false;
            default:
                word.push_back(ch);
                state = INW;
                break;
            }
            break;
        case EOS:
            switch (ch)
            {
            case ' ': case '\t': case '\r':
            case '\v': case '\n': case '\f':
                break;
            case EOF:
                state = EOF_S;
                return false;
            default:
                ungetc(ch, fin);
                --pos;
                actual = &sos;
                state = OUTW;
                return true;
            }
            break;
        case EOF_S:
            return false;
    }
}

const std::string& Reader::GetItem() const
{
    return *actual;
}

const size_t& Reader::GetPosition() const
{
    return pos;
}
