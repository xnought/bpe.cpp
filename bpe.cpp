#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <omp.h>

using namespace std;

template <typename K, typename V>
void merge_maps(unordered_map<K, V> &dst, unordered_map<K, V> &src)
{
	for (auto const &[key, val] : src)
	{
		if (dst.find(key) != dst.end())
		{
			dst[key] += src[key];
		}
		else
		{
			dst[key] = src[key];
		}
	}
}

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
	for (int j = new_vocab.size() - 1; j >= 0; j--)
	{
		string v = new_vocab[j];
		bool fits = (i + v.size()) <= s.size();
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

void insert_sorted_str_size(vector<string> &v, string &s)
{
	if (v.size() == 0)
	{
		v.push_back(s);
		return;
	}

	for (int i = v.size() - 1; i >= 0; i--)
	{
		string item = v[i];
		if (s.size() >= item.size())
		{
			v.insert(v.begin() + i + 1, s);
			return;
		}
	}
}

template <typename K, typename V>
bool map_has(unordered_map<K, V> &m, K &check)
{
	return m.find(check) != m.end();
}

template <typename K>
void increment_frequency(unordered_map<K, size_t> &m, K &key)
{
	if (!map_has(m, key))
		m[key] = 1;
	else
		m[key]++;
}

void experiments()
{
	string s[1] = {"hi there my name is donny"};
	vector<string> vocab = {"do", "hi", "hi ", "hi t"};
	string a = "hi there";
	insert_sorted_str_size(vocab, a);

	vector<size_t> idxs = token_idxs(s[0], vocab);

	for (size_t i = 0; i < idxs.size() - 1; i++)
	{
		printf("(%zu, %zu) ", idxs[i], idxs[i + 1]);
		printf("%s\n", substr_between(s[0], idxs[i], idxs[i + 1]).c_str());
	}

	unordered_map<string, size_t> map;
	map[a] = 20;
	if (map_has(map, a))
	{
		printf("%lu\n", map[a]);
	}

	increment_frequency(map, a);

	if (map_has(map, a))
	{
		printf("%lu\n", map[a]);
	}
}

void print_vocab(vector<string> &v)
{
	for (int i = 0; i < v.size(); i++)
	{
		printf("'%s'\n", v[i].c_str());
	}
}

void print_counter(unordered_map<string, size_t> &counter)
{
	for (auto const &[key, val] : counter)
	{
		printf("'%s' %zu\n", key.c_str(), val);
	}
}

string find_most_frequent(unordered_map<string, size_t> &counter)
{
	size_t max_val = counter.begin()->second;
	string max_str = counter.begin()->first;

	for (auto const &[key, val] : counter)
	{
		if (val > max_val)
		{
			max_val = val;
			max_str = key;
		}
	}

	return max_str;
}

vector<string> train_bpe_parallel(string strings[], size_t n_strings, size_t n_iter)
{
	vector<string> vocab;
	vocab.reserve(n_iter);

	int total_threads = omp_get_num_procs();
	printf("Total number of threads: %d\n", total_threads);
	int data_per_thread = (int)ceil((float)n_strings / (float)total_threads);
	printf("Data (n_strings) processing per thread %d\n", data_per_thread);

	omp_lock_t writelock;
	omp_init_lock(&writelock);

	// outer training loop
	for (size_t i = 0; i < n_iter; i++)
	{
		printf("%zu/%zu\n", i, n_iter);
		unordered_map<string, size_t> global_counter;
#pragma omp parallel
		{

			int gi = omp_get_thread_num();
			// count the frequencies of byte pairs for each string
			unordered_map<string, size_t> counter;
			for (size_t j = 0; j < data_per_thread; j++)
			{
				size_t gj = gi * data_per_thread + j;
				if (gj < n_strings)
				{
					string s = strings[gj];
					// iterate over the string given the vocab merges
					vector<size_t> idxs = token_idxs(s, vocab);
					for (size_t k = 0; k < idxs.size() - 2; k++)
					{
						// TODO: figure out how to not copy strings so many times here
						string tok1 = substr_between(s, idxs[k], idxs[k + 1]);
						string tok2 = substr_between(s, idxs[k + 1], idxs[k + 2]);
						string pair = tok1 + tok2;
						increment_frequency(counter, pair);
					}
				}
			}

			omp_set_lock(&writelock);
			merge_maps(global_counter, counter);
			omp_unset_lock(&writelock);
		}

		// if there is nothing to count, we are done done done
		if (global_counter.size() == 0)
			goto done;

		// add most frequent pair to the merge rules/vocab
		string most_freq = find_most_frequent(global_counter);
		insert_sorted_str_size(vocab, most_freq);
	}

done:
	omp_destroy_lock(&writelock);
	return vocab;
}

vector<string> train_bpe(string strings[], size_t n_strings, size_t n_iter)
{
	vector<string> vocab;
	vocab.reserve(n_iter);

	// outer training loop
	for (size_t i = 0; i < n_iter; i++)
	{
		// count the frequencies of byte pairs for each string
		unordered_map<string, size_t> counter;
		for (size_t j = 0; j < n_strings; j++)
		{
			string s = strings[j];
			// iterate over the string given the vocab merges
			vector<size_t> idxs = token_idxs(s, vocab);
			for (size_t k = 0; k < idxs.size() - 2; k++)
			{
				// TODO: figure out how to not copy strings so many times here
				string tok1 = substr_between(s, idxs[k], idxs[k + 1]);
				string tok2 = substr_between(s, idxs[k + 1], idxs[k + 2]);
				string pair = tok1 + tok2;
				increment_frequency(counter, pair);
			}
		}

		// if there is nothing to count, we are done done done
		if (counter.size() == 0)
			return vocab;

		// add most frequent pair to the merge rules/vocab
		string most_freq = find_most_frequent(counter);
		insert_sorted_str_size(vocab, most_freq);
	}

	return vocab;
}

void bpe_example()
{
	string s[1] = {"hi there my name is donny"};
	size_t n_iter = 4;
	vector<string> v = train_bpe(s, 1, n_iter);
}

string gen_random_string(size_t len)
{
	string s;
	s.reserve(len);
	for (size_t i = 0; i < len; i++)
	{
		char rand_char = 'a' + (rand() % 26);
		s.push_back(rand_char);
	}
	return s;
}

#define N_STRINGS 300000
#define STR_LEN 256
#define N_ITER 10
void bpe_larger_example()
{
	// Generate N_STRINGS with random letters of length STR_LEN each
	string s[N_STRINGS];
	srand(0);
	for (size_t i = 0; i < N_STRINGS; i++)
	{
		s[i] = gen_random_string(STR_LEN);
	}
	vector<string> v = train_bpe(s, N_STRINGS, N_ITER);
}
void bpe_larger_example_parallel()
{
	// Generate N_STRINGS with random letters of length STR_LEN each
	string s[N_STRINGS];
	srand(0);
	for (size_t i = 0; i < N_STRINGS; i++)
	{
		s[i] = gen_random_string(STR_LEN);
	}
	vector<string> v = train_bpe_parallel(s, N_STRINGS, N_ITER);
}

size_t num_lines(ifstream &f)
{
	string line;
	size_t count = 0;
	while (getline(f, line))
		count++;
	f.clear();
	f.seekg(0);
	return count;
}

/**
 * Reads in the file into array of strings
 * returns NULL if fails, returns string * that needs to be freed otherwise with delete[]
 */
string *parse_file(string filename, size_t &lines)
{
	ifstream f;
	f.open(filename);
	if (!f.is_open())
		return NULL;

	lines = num_lines(f);

	string *parsed = new string[lines];
	string line;
	size_t i = 0;
	while (getline(f, line))
	{
		parsed[i] = line;
		i++;
	}

	f.close();
	return parsed;
}

void write_vocab(string filename, vector<string> &vocab)
{
	ofstream f;
	f.open(filename);

	if (!f.is_open())
		return;

	for (size_t i = 0; i < vocab.size(); i++)
	{
		f << vocab[i] << "\n";
	}

	f.close();
}

void bpe_file_example()
{
	size_t data_len;
	string *data = parse_file("data.txt", data_len);
	if (data == NULL)
		return;

	vector<string> res = train_bpe(data, data_len, 10);

	write_vocab("data_vocab.txt", res);

	delete[] data;
}

void bpe_titles()
{
	size_t data_len;
	string *data = parse_file("experiments/titles.txt", data_len);
	if (data == NULL)
		return;

	printf("Loaded %zu rows of strings\n", data_len);

	printf("Training BPE\n");
	vector<string> res = train_bpe_parallel(data, data_len, 16);

	printf("Saving result\n");
	write_vocab("experiments/titles_vocab.txt", res);

	delete[] data;
}

int main()
{
	// experiments();
	// bpe_example();
	// bpe_larger_example();
	// bpe_larger_example_parallel();
	// bpe_file_example();
	bpe_titles();
}