#include <iostream>
#include <string>

bool isvalid(char character)
{
	if (isspace(character) || character == '.' || character == '!' || character == '?')
	{
		return true;
	}
	else
	{
		return false;
	}
}

void add_numbers_from_cin(std::istream& input)
{
	int sum = 0;
	char character;
	std::string string_number;
	char char_before = '.';
	while (true)
	{
		character = input.get();
		if (input.fail())
		{
			if (!string_number.empty() && isvalid(char_before))
			{
				int int_number = std::stoi(string_number);
				sum += int_number;
			}

			std::cout << sum << std::endl;

			return;
		}

		if (isvalid(character))
		{
			if (!string_number.empty() && isvalid(char_before))
			{
				int int_number = std::stoi(string_number);
					sum += int_number;
					string_number = "";
			}
			else
			{
				char_before = character;
			}
		}
		else if (isdigit(character))
		{
			if (isvalid(char_before))
			{
				string_number.push_back(character);
			}
		}
		else
		{
			char_before = character;
			string_number = "";
		}
	}
}

int main()
{
	add_numbers_from_cin(std::cin);

	return 0;
}