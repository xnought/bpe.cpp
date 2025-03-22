#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

#define uint unsigned int

/**
 * Does the string in a (offset with a_offset) start with the string in b?
 */
bool starts_with(uint a_offset, string &a, string &b)
{
	size_t asize = a.size() - a_offset;
	size_t bsize = b.size();

	if (bsize > asize)
	{
		return false;
	}
	else
	{
		for (size_t i = 0; i < bsize; i++)
		{
			if (a[i + a_offset] != b[i])
				return false;
		}
		return true;
	}
}

uint find_slice_match(uint i, string &s, vector<string> &new_vocab)
{
	for (uint j = 0; j < new_vocab.size(); j++)
	{
		string v = new_vocab[j];
		bool fits = (i + v.size()) < s.size();
		if (fits && starts_with(i, s, v))
		{
			return v.size();
		}
	}
	return 1;
}

vector<uint> token_idxs(string &s, vector<string> &new_vocab)
{
	vector<uint> token_idxs = {0};

	uint i = 0;
	while (i < s.size())
	{
		uint slice_len = find_slice_match(i, s, new_vocab);
		token_idxs.push_back(i + slice_len);
		i += slice_len;
	}

	return token_idxs;
}

int main()
{
	string s[2] = {"hello", "world"};
	vector<string> vocab = {"hell", "he"};
	vector<uint> idxs = token_idxs(s[0], vocab);

	for (uint i = 0, j = 1; i < idxs.size() - 1; i++, j++)
	{
		printf("%s\n", s[0].substr(idxs[i], idxs[j]).c_str());
	}
}