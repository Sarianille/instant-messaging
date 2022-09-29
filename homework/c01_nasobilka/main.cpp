#include <iostream>
#include <vector>
#include <string>

using namespace std;

void write_multiplications(vector<int> numbers, int from, int to)
{
	for (auto&& number : numbers)
	{
		for (int i = from; i <= to; i++)
		{
			int multiplication_result = i * number;
			cout << number << " * " << i << " = " << multiplication_result << endl;
		}
	}
}

void define_range(vector<string>& args, int number_of_arguments)
{
	int from = 1;
	int to = 10;
	vector<int> numbers;

	for (int i = 0; i < number_of_arguments; i++)
	{
		if (args[i] == "-f")
		{
			from = stoi(args[i + 1]);
			i++;
		}
		else if (args[i] == "-t")
		{
			to = stoi(args[i + 1]);
			i++;
		}
		else
		{
			numbers.push_back(stoi(args[i]));
		}
	}

	write_multiplications(numbers, from, to);
}

int main(int argc, char** argv)
{
	vector<string> args(argv + 1, argv + argc);
	int number_of_arguments = args.size();

	define_range(args, number_of_arguments);
}