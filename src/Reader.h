#ifndef INCLUDE_READER_H
#define INCLUDE_READER_H

#include <limits.h>
#include <stdio.h>

#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>
#include <list>

// due to portability issues
#ifdef _MSC_VER
#define nprintf sprintf_s
#elif defined __GNUC__
#define nprintf snprintf
#endif // _MSC_VER

template<class T>
struct Format
{
    static const char* str;
    typedef T Got;
    static const Got& get(const T& t){ return t; }
};

template<>
struct Format<std::string>
{
    static const char* str;
    typedef const char* Got;
    static Got get(const std::string& s){ return s.c_str(); }
};

class Reader
{
public:
    Reader(FILE*, std::string sos="<s>", std::string eos="</s>");
    bool ReadNext();
    const std::string& GetItem()const;
    const size_t& GetPosition()const;
    typedef std::string Type;
private:
    enum State
    {
        INW,  //!< inside a word
        OUTW, //!< outside a word
        EOS,  //!< end-of-sentence
        EOS_P,//!< end-of-sentence is pending, next call will result EOS symbol
        EOF_S //!< end-of-file symbol has been reached
    } state;
    FILE* const fin;
    size_t pos;
    std::string word;
    std::string* actual;
    std::string sos;
    std::string eos;
};

typedef std::unordered_map<std::string, size_t> Vocabulary;

//! reads a vocabulary file into a Vocabulary instance
/*!
	mind the hard-coded word length limit, see .cpp file!
*/
Vocabulary ReadVocab(FILE* fin);

//! build vocabulary from a corpus file
/*!
	@param count cutoff
*/
void CreateVocab(FILE*, Vocabulary& vocab, long long count = 0);

class IndexReader : public Reader
{
public:
    IndexReader(FILE*, const Vocabulary& v, std::string unk_token);
    size_t GetItem()const;
    typedef size_t Type;
private:
    size_t GetIndex(const std::string& w)const;
    const Vocabulary* const _w2i;
    Vocabulary::const_iterator _end;
    const std::string _unk;
    const size_t _unk_index;
};

template<class R>
class WindowsReader
{
    typedef typename R::Type T;
    typedef std::vector<T> Type;
public:
    WindowsReader(R* reader, unsigned int window_l, unsigned int window_r, std::string space, std::string place, T sos, T eos, bool pan_out = false)
        : _reader(reader), _word(), _window_l(window_l), _window_r(window_r), _eos(eos), _sos(sos), pan(pan_out), pad(!place.empty())
    {
        BeginSentence();
        nprintf(format1, 1023, "%s%s", Format<T>::str, space.c_str());
        nprintf(format2, 1023, "%s%s", space.c_str(), Format<T>::str);

        nprintf(placeholder1, 1023, "%s%s", place.c_str(), space.c_str());
        nprintf(placeholder2, 1023, "%s%s", space.c_str(), place.c_str());
    }
    //! slides the windows with one
    void ReadItem()
    {
        if (pan ? (_discourse[_word] == _eos) : (_discourse.back() == _eos))
            return BeginSentence();
        if (_discourse.back() != _eos)
            GetNext();

        if (_word == _window_l)
        {
            _discourse.erase(_discourse.begin());
            _positions.erase(_positions.begin());
        }
        else
            ++_word;
    }
    void Print(FILE* fout)const
    {
        static auto const format = format2 + 1;
        size_t i;
        if (pad)
        {
            // printf is faster than std::cout
            for (i = 0; i < _window_l - _word; ++i)
                fputs(placeholder1, fout);
        }
        for (i = 0; i < _word; ++i)
            fprintf(fout, format1, Format<T>::get(_discourse[i]));
        if (_discourse.size() > _word)
            fprintf(fout, format, Format<T>::get(_discourse[_word]));
        for (i = _word + 1; i < _discourse.size(); ++i)
            fprintf(fout, format2, Format<T>::get(_discourse[i]));
        if (pad)
        {
            // little trick here, i < n + 1 is not the same as i <= n, because of the unsigned underflow
            for (i = 0; i < _window_r + _word + 1 - _discourse.size(); ++i)
                fputs(placeholder2, fout);
        }
        fprintf(fout, "\n");
    }
    bool IsGood()const
    {
        return !_discourse.empty();
    }
    const Type& GetContext()const{ return _discourse; }
    const T& GetWord()const{ return _word; }
    const size_t& GetPosition()const{ return _positions[_word]; }
private:
    bool GetNext()
    {
        auto const result = _reader->ReadNext();
        if (result)
        {
            _discourse.emplace_back(_reader->GetItem());
            _positions.emplace_back(_reader->GetPosition());
        }
        return result;
    }
    R* const _reader;
    //! contains the whole window
    Type _discourse;
    //! seek position of the file at each word of the discourse
    std::vector<size_t> _positions;
    //! position of the current word in the discourse
    size_t _word;
    const unsigned int _window_l;
    const unsigned int _window_r;
    const T _eos;
    const T _sos;
    char placeholder1[1024];
    char placeholder2[1024];
    char format1[1024];
    char format2[1024];
    char format[1024];
    const bool pan;
    const bool pad;

    void BeginSentence()
    {
        static const unsigned int w = pan ? _window_r : _window_l + _window_r;
        _discourse.clear();
        _positions.clear();
        _word = 0;
        while (_discourse.size() <= w && GetNext())
        {
            _word = std::max<long long>(0, _discourse.size() - 1 - _window_r);
            if (_reader->GetItem() == _eos)
                return;
        }
    }
};

#endif // !INCLUDE_READER_H