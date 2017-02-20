#include <algorithm>
#include <utility>
#include <iostream>

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

/*!
see Dmitry Andreev's (https://github.com/unipolar) cpp implementation in https://github.com/juditacs/wordcount
*/
void CreateVocab(FILE* fin, Vocabulary& vocab, long long count)
{
	typedef std::vector<std::tuple<long long, std::string>> SortedVocab;

	Reader reader(fin);

	vocab.clear();

	while (reader.ReadNext())
	{
		++vocab[reader.GetItem()];
	}

	SortedVocab sorted(vocab.size());
	auto sorted_it = sorted.begin();
	for (auto vocab_it = vocab.begin(); vocab_it != vocab.end(); ++vocab_it, ++sorted_it)
	{
		std::get<0>(*sorted_it) = -(long long)(vocab_it->second);
		std::get<1>(*sorted_it) = vocab_it->first;
	}

	std::sort(sorted.begin(), sorted.end());
	vocab.clear();

	auto end = sorted.begin();
	if (count > 0)
		std::advance(end, std::min<size_t>(sorted.size(), count));
	else if (count < 0)
		end = std::lower_bound(sorted.begin(), sorted.end(), SortedVocab::value_type(count + 1, ""));
	else
		end = sorted.end();

	size_t i = 0;
	for (auto it = sorted.begin(); it < end; ++it, ++i)
	{
		vocab[std::get<1>(*it)] = i;
	}
}

Vocabulary ReadVocab(FILE* fin)
{
	Vocabulary V;
	char* word = new char[1024];
	size_t j = 0;

	if (fin)
		while (fscanf(fin, "%1023[^ \t\r\n\v\f] %*[^\n]\n", word) == 1)
		{
			V[word] = j++;
		}

	delete[] word;
	return V;
}

size_t IndexReader::GetIndex(const std::string& w) const
{
	auto where = _w2i->find(w);
	return where == _end ? _unk_index : where->second;
}

//long long IndexReader::GetIndex()
//{
//    return ReadNext() ? GetIndex(static_cast<const Reader*>(this)->GetItem()) : -1;
//}

IndexReader::IndexReader(FILE* fin, const Vocabulary& v, std::string unk_token)
    : Reader(fin), _w2i(&v), _unk(unk_token), _unk_index(v.at(unk_token))
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
    int ch;
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
        case EOS_P:
            state = EOS;
            actual = &eos;
            return true;
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
