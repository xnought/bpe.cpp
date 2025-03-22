#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

/**
 * Does the string in a (offset with a_offset) start with the string in b?
 */
bool starts_with(size_t a_offset, string &a, string &b)
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

size_t find_slice_match(size_t i, string &s, vector<string> &new_vocab)
{
	for (size_t j = 0; j < new_vocab.size(); j++)
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

vector<size_t> token_idxs(string &s, vector<string> &new_vocab)
{
	vector<size_t> token_idxs = {0};

	size_t i = 0;
	while (i < s.size())
	{
		size_t slice_len = find_slice_match(i, s, new_vocab);
		i += slice_len;
		token_idxs.push_back(i);
	}
	return token_idxs;
}

string substr_between(string &s, size_t a, size_t b)
{
	return s.substr(a, b - a);
}

int main()
{
	string s[1] = {"hi there my name is donny"};
	vector<string> vocab = {"hi t", "hi ", "hi", "do"};
	vector<size_t> idxs = token_idxs(s[0], vocab);

	for (size_t i = 0; i < idxs.size() - 1; i++)
	{
		printf("(%zu, %zu) ", idxs[i], idxs[i + 1]);
		printf("%s\n", substr_between(s[0], idxs[i], idxs[i + 1]).c_str());
	}
}