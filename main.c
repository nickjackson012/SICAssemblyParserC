#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


const int STRING_TERMINATOR_LENGTH = 1;
const int MAX_LINE_LENGTH = 256 + STRING_TERMINATOR_LENGTH;
const int MAX_LABEL_LENGTH = 6 + STRING_TERMINATOR_LENGTH;
const int MAX_OPCODE_LENGTH = 5 + STRING_TERMINATOR_LENGTH;
const int MAX_OPERAND_LENGTH = 35 + STRING_TERMINATOR_LENGTH;
const int NUMBER_OF_OPCODES = 35;
const int NUMBER_OF_LONE_OPCODES = 3;
const int MAXIMUM_LENGTH_OF_START_OPERAND = 4;
const int MAXIMUM_MEMORY_ADDRESS_DEC = 32767;
const int MINIMUM_MEMORY_ADDRESS_DEC = 0;
const int MAXIMUM_BYTE_OPCODE_LENGTH = 32;
const int MINIMUM_BYTE_OPCODE_LENGTH = 1;
const int MINIMUM_INTEGER = -8388608;
const int MAXIMUM_INTEGER = 8388607;
const int MINIMUM_RESB = 1;
const int MAXIMUM_RESB = 32768;
const int MINIMUM_RESW = 1;
const int MAXIMUM_RESW = 10922; // MAXIMUM_RESB / 3
const char DEFAULT_STRING[] = "\0";
const char WHITESPACE[] = " \t\n\r\v\f";
const int STRING_TO_INT_ERROR_INDICATOR = -9000000;


struct parsed_line_of_code_struct{
	int line_number;
	bool is_comment;
	bool is_index_addressing;
	char label_str[MAX_LABEL_LENGTH];
	char opcode_str[MAX_OPCODE_LENGTH];
	char operand_str[MAX_OPERAND_LENGTH];
	char unparsed_line_of_code_str[MAX_LINE_LENGTH];
};

struct parsed_line_of_code_struct* create_parsed_line_of_code_struct(int line_number,
																	 char unparsed_line_of_code_str[MAX_LINE_LENGTH]){
	struct parsed_line_of_code_struct *parsed_line_of_code_struct_ptr = (struct parsed_line_of_code_struct*) malloc(sizeof(struct parsed_line_of_code_struct));
	parsed_line_of_code_struct_ptr->line_number = line_number;
	parsed_line_of_code_struct_ptr->is_comment = false;
	parsed_line_of_code_struct_ptr->is_index_addressing = false;
	strcpy(parsed_line_of_code_struct_ptr->label_str, DEFAULT_STRING);
	strcpy(parsed_line_of_code_struct_ptr->opcode_str, DEFAULT_STRING);
	strcpy(parsed_line_of_code_struct_ptr->operand_str, DEFAULT_STRING);
	strcpy(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str, unparsed_line_of_code_str);

	return parsed_line_of_code_struct_ptr;
}


// START OF TRIM FUNCTIONS
char *ltrim(char *s){
	while(isspace(*s)) s++;
	return s;
}

char *rtrim(char *s){
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *trim(char *s){
	return rtrim(ltrim(s));
}
// END OF TRIM FUNCTIONS

// If the trimmed line of code is empty, then the method will return 0 (false)
bool test_for_blank_line(char *unparsed_line_of_code_str){
	char* trimmed_line_of_code = trim(unparsed_line_of_code_str);

	return !strlen(trimmed_line_of_code);
}

// This method checks for the comment line indicator ('.') as the first character in the trimmed line of code.
bool test_for_comment_line(char *unparsed_line_of_code_str){
	char* trimmed_line_of_code = ltrim(unparsed_line_of_code_str);

	return trimmed_line_of_code[0] == '.';
}

// This method checks for the presence of a character in the 0 index of the line of code
bool test_for_label(const char *unparsed_line_of_code_str){
	return !isblank(unparsed_line_of_code_str[0]);
}

// This function parses and returns a BYTE character string
// if one exists in the line of code, otherwise it returns null.
char* get_byte_character_string(const char *unparsed_line_of_code_str){
	char* temp_string = strstr(unparsed_line_of_code_str, "C\'");
	if(temp_string != NULL){
		temp_string += 2;
		char *end_string = strchr(temp_string, '\'');
		if(end_string != NULL){
			char byte_character_string[MAX_LINE_LENGTH];
			int end_index = (int) (end_string - temp_string);
			strncpy(byte_character_string, temp_string, end_index);
			byte_character_string[end_index] = '\0';
			char* formatted_byte_character_string = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
			sprintf(formatted_byte_character_string, "%s%s%s", "C\'", byte_character_string, "\'");
			return formatted_byte_character_string;
		}
	}
	return NULL;
}

struct split_token_array_struct{
	char** token_array;
	int length;
};

struct split_token_array_struct* split_line_of_code(const char *raw_unparsed_line_of_code_str){
	char* unparsed_line_of_code_str = malloc(sizeof(char) * MAX_LINE_LENGTH);
	strcpy(unparsed_line_of_code_str, raw_unparsed_line_of_code_str);
	struct split_token_array_struct *return_ptr = malloc(sizeof(struct split_token_array_struct));
	return_ptr->token_array = malloc(3 * sizeof(char));
	char *token = strtok(unparsed_line_of_code_str, WHITESPACE);
	int index = 0;
	while (token != NULL && index < 3){
		return_ptr->token_array[index] = token;
		token = strtok(NULL, WHITESPACE);
		index++;
	}
	return_ptr->length = index;
	return return_ptr;
}

// This function handles BYTE character strings.
// This special handling is required when there are
// spaces in the BYTE character string that would
// be tokenized otherwise.
void handle_byte_character_string(struct split_token_array_struct* token_array_struct_ptr, char* byte_character_string){
	int byte_opcode_index = -1;
	for (int i = 0; i < token_array_struct_ptr->length; i++){
		if (strcmp(token_array_struct_ptr->token_array[i], "BYTE") == 0){
			byte_opcode_index = i;
		}
	}

	if (byte_opcode_index == 0 && token_array_struct_ptr->length == 2){
		char* s = strstr(token_array_struct_ptr->token_array[byte_opcode_index + 1], "C\'");
		if (s != NULL){
			strcpy(token_array_struct_ptr->token_array[byte_opcode_index + 1], byte_character_string);
		}
	}

	if (byte_opcode_index == 1 && token_array_struct_ptr->length == 3){
		char* s = strstr(token_array_struct_ptr->token_array[byte_opcode_index + 1], "C\'");
		if (s != NULL){
			strcpy(token_array_struct_ptr->token_array[byte_opcode_index + 1], byte_character_string);
		}
	}
}

// This function validates label tokens against defined label rules:
// 1) Labels can only contain uppercase (A-Z) and numeric (0-9) characters
// 2) The first character in a label must be uppercase (A-Z)
// 3) Labels must be 1 to 6 characters in length
char* validate_label(char* label_str){
	int label_str_length = strlen(label_str);
	bool length_valid = label_str_length > 0 && label_str_length <= 6;
	bool first_character_valid = isalpha(label_str[0]) && isupper(label_str[0]);
	bool characters_valid = true;
	for (int i = 0; i < label_str_length; i++){
		// Test for invalid character
		if (islower(label_str[i]) || !isalnum(label_str[i])){
			characters_valid = false;
		}
	}

	if (length_valid && first_character_valid && characters_valid){
		// Label is valid
		return label_str;
	}

	if (length_valid == false){
		printf("ERROR: Label must be 1-6 characters in length.\n");
	}
	if (first_character_valid == false){
		printf("ERROR: The first character in a label must be uppercase(A-Z).\n");
	}
	if (characters_valid == false){
		printf("ERROR: Labels can only contain uppercase (A-Z) and numeric (0-9) characters.\n");
	}

	//Invalid label
	return NULL;
}

// This function validates opcode tokens against a set of valid opcodes.
char* validate_opcode(char* opcode_str){
	char OPCODE_VALIDATION_ARRAY[NUMBER_OF_OPCODES][MAX_OPCODE_LENGTH] = {
								"ADD", "AND", "COMP", "DIV", "J", "JEQ", "JGT",
                                "JLT", "JSUB", "LDA", "LDCH", "LDL", "LDX", "MUL",
                                "OR", "RD", "RSUB", "STA", "STCH", "STL", "STSW",
                                "STX", "SUB", "TD", "TIX", "WD", "START", "END",
                                "BYTE", "WORD", "RESB", "RESW", "XOS", "TIXB", "TIXW"
	};
	for (int i = 0; i <= NUMBER_OF_OPCODES; i++){
		if (strcmp(opcode_str, OPCODE_VALIDATION_ARRAY[i]) == 0){
			// Opcode valid
			return opcode_str;
		}
	}

	// Opcode not valid
	printf("ERROR: Opcode is invalid.\n");
	return NULL;
}

// This function validates lone opcode tokens against a set of valid lone opcodes.
char* validate_lone_opcode(char* opcode_str){
	char LONE_OPCODE_VALIDATION_ARRAY[NUMBER_OF_LONE_OPCODES][MAX_OPCODE_LENGTH] = {"RSUB", "END", "XOS"};
	for (int i = 0; i <= NUMBER_OF_LONE_OPCODES; i++){
		if (strcmp(opcode_str, LONE_OPCODE_VALIDATION_ARRAY[i]) == 0){
			// Opcode valid
			return opcode_str;
		}
	}

	// Opcode not valid
	printf("ERROR: Lone opcode is invalid.\n");
	return NULL;
}

int hex_str_to_dec(char* hex_str){
	int dec_value = 0;
	for (int i = strlen(hex_str) - 1; i >= 0; i--){
		int temp_dec_value;
		if (isdigit(hex_str[i])){
			temp_dec_value = hex_str[i] - '0';
		}
		else{
			switch (hex_str[i]){
				case 'A':
					temp_dec_value = 10;
					break;
				case 'B':
					temp_dec_value = 11;
					break;
				case 'C':
					temp_dec_value = 12;
					break;
				case 'D':
					temp_dec_value = 13;
					break;
				case 'E':
					temp_dec_value = 14;
					break;
				case 'F':
					temp_dec_value = 15;
					break;
				default:
					printf("ERROR: Invalid hex digit.\n");
					return -1;
			}
		}
		dec_value += temp_dec_value * pow(16, ((strlen(hex_str) - 1) - i));
	}
	return dec_value;
}

long int str_to_int(char* character_string){
	char* endptr;
	long int num;

	num = strtol(character_string, &endptr, 10);

	if (endptr == character_string) {
		printf("ERROR: No digits were found in str_to_int().\n");
		num = STRING_TO_INT_ERROR_INDICATOR;
	}

	else if (*endptr != '\0'){
		printf("ERROR: Invalid character in str_to_int(): %c\n", *endptr);
		num = STRING_TO_INT_ERROR_INDICATOR;
	}

	return num;
}

// This method will add a character to the beginning of a string
void prepend_char(char* string, char character){
	size_t string_length = strlen(string);
	memmove(string + 1, string, string_length + 1);
	string[0] = character;
}

// This function validates the START operand.
// RULES:
//      1) Operand must be a valid memory address hex string (0000 - 7FFF).
//         No additional decoration should be present.
char* validate_start_operand(char* operand_str){
	char error_message[] = "ERROR: Start address must be between 0000-7FFF.\n";
	if (strlen(operand_str) > MAXIMUM_LENGTH_OF_START_OPERAND){
		printf("%s", error_message);
		return NULL;
	}

	int start_address_dec = hex_str_to_dec(operand_str);
	if (start_address_dec == -1){
		printf("%s", error_message);
		return NULL;
	}

	if (start_address_dec < MINIMUM_MEMORY_ADDRESS_DEC || start_address_dec > MAXIMUM_MEMORY_ADDRESS_DEC){
		printf("%s", error_message);
		return NULL;
	}

	int required_leading_zeros = MAXIMUM_LENGTH_OF_START_OPERAND - strlen(operand_str);

	for (int i = 0; i < required_leading_zeros; i++){
		prepend_char(operand_str, '0');
	}
	return operand_str;
}

// This method validates the BYTE operand.
// RULES:
//      1) Operand can be either a hex string or a string or characters
//      2A.) If operand is a hex string it must be formed as such "X'hhhh...'" (h = valid hex digit string)
//      2B.) The hex digit string must contain an even number of characters and must be 2-32 characters in length.
//      3A.) If operand is a character string it must be formed as such "C'aaaa...'" (a = valid ascii character)
//      3B.) The character string must contain an even number of characters and must be  1-32 characters in length.
char* validate_byte_operand(char* operand_str){
	// Test for valid string length
	// BYTE operands are indicated by X'BYTE_OPERAND' or C'BYTE_OPERAND'
	// We subtract 3 from the operand length to get the length of the
	// characters between the quotation marks
	int byte_operand_length = (strlen(operand_str) - 3);

	if (strlen(operand_str) < 4){
		printf("ERROR: Invalid BYTE operand\n");
		return NULL;
	}

	if (operand_str[1] == '\'' && operand_str[strlen(operand_str) - 1] == '\''){
		if (byte_operand_length < MINIMUM_BYTE_OPCODE_LENGTH || byte_operand_length > MAXIMUM_BYTE_OPCODE_LENGTH){
			printf("OPERAND must have 1 - 32 characters between quotation marks. \n");
			return NULL;
		}
	}

	else{
		printf("ERROR: Single quotation marks missing from OPERAND.\n");
		return NULL;
	}

	// Verify hex BYTE operand
	if (operand_str[0] == 'X'){
		char hex_digit_str[33];
		memcpy(hex_digit_str, &operand_str[2], byte_operand_length);
		hex_digit_str[byte_operand_length] = '\0';

		// Test for an even number of hex digits within the BYTE operand
		if ((strlen(hex_digit_str) % 2) == 0){
			for (int i = 0; i < strlen(hex_digit_str); i++){
				if (!isxdigit(hex_digit_str[i])){
					printf("ERROR: OPERAND must contain a valid hex value.\n");
					return NULL;
				}
			}
			// Valid hex BYTE Operand
			return operand_str;
		}
		else{
			printf("ERROR: OPERAND must contain an even number of hex digits.\n");
			return NULL;
		}
	}

	// Verify character BYTE operand
	else if (operand_str[0] == 'C'){
		char character_str[33];
		memcpy(character_str, &operand_str[2], byte_operand_length);
		character_str[byte_operand_length] = '\0';

		// Test for ASCII characters
		for(int i = 0; i < strlen(character_str); i++){
			if (character_str[i] < 0 || character_str[i] > 127){
				printf("ERROR: OPERAND must contain ASCII characters.\n");
				return NULL;
			}
		}
		// Valid Character operand
		return operand_str;
	}

	else{
		printf("ERROR: Valid format indicator (C or X) required in OPERAND.\n");
		return NULL;
	}
}

// This function validates the WORD operand.
// RULES:
//  1.) Operand can be a decimal integer in the supported SIC architecture range.
//        -8,388,608 to 8,388,607
char* validate_word_operand(char* operand_str){
	long int word_int_value = str_to_int(operand_str);

	if (word_int_value < MINIMUM_INTEGER || word_int_value > MAXIMUM_INTEGER){
		printf("ERROR: WORD operand must be a decimal integer in the range of -8,388,608 and 8,388,607. \n");
		return NULL;
	}

	return operand_str;
}

// This function validates the RESB operand.
// RULES:
// NOTE: this is a very loose restriction. It will not protect against reserving more memory than what is available.
//    1.) Operand must be a positive decimal integer
//    2.) Operand must be less than the number of possible bytes in memory(32,768).
char* validate_resb_operand(char* operand_str){
	long int resb_int_value = str_to_int(operand_str);

	if (resb_int_value < MINIMUM_RESB || resb_int_value > MAXIMUM_RESB){
		printf("ERROR: RESB operand must be a decimal integer in the range of 1 and 32,768. \n");
		return NULL;
	}

	return operand_str;
}

// This function validates the RESW operand.
// RULES:
// NOTE: this is a very loose restriction. It will not protect against reserving more memory than what is available.
//    1.) Operand must be a positive decimal integer
//    2.) Operand must be less than the number of possible words in memory(10,922).
char* validate_resw_operand(char* operand_str){
	long int resw_int_value = str_to_int(operand_str);

	if (resw_int_value < MINIMUM_RESW || resw_int_value > MAXIMUM_RESW){
		printf("ERROR: RESB operand must be a decimal integer in the range of 1 and 10,922. \n");
		return NULL;
	}

	return operand_str;
}

struct validate_nonspecific_operand_struct{
	char operand_str[MAX_OPERAND_LENGTH];
	bool is_indexed_addressing;
};

bool g_is_indexed_addressing = false;

// This method validates the operand for all other opcodes.
// RULES:
//    1.) Operand can be a string that follows label rules (see validate_label() for label rules)
//    2.) Operand can be a valid hex memory address string padded with "0" as the first character.
//           a max of 5 total characters ex. "0hhhh" (h = valid hex value)
//    3.) Each of the above may include the indexed addressing indicator(",X")
//    as the last two characters in the string
char* validate_nonspecific_operand(char* operand_str) {
	g_is_indexed_addressing = false;

	// Check for index addressing and remove the indicator (,X) from the operand.
	int length = strlen(operand_str);
	char index_addressing_substr[3];
	// Copy 3 bytes in order to get last
	// two characters plus the null indicator
	memcpy(index_addressing_substr, &operand_str[length - 2], 3);
	char* temp_operand_str = malloc(sizeof(MAX_OPERAND_LENGTH));
	strcpy(temp_operand_str, operand_str);
	if (strcmp(index_addressing_substr, ",X") == 0) {
		g_is_indexed_addressing = true;
		temp_operand_str[length - 2] = '\0';
	}

	// If the first character is alphabetical,
	// we expect the operand to be a label.
	if (isalpha(temp_operand_str[0])){
		if (validate_label(temp_operand_str) == NULL) {
			free(temp_operand_str);
			printf("ERROR: Operand must be formatted as a label.\n");
			return NULL;
		}
		return temp_operand_str;
	}

	else if (temp_operand_str[0] == '0') {
		char error_message[] = "Operand memory address must be between 00000 - 07FFF.";
		if (strlen(temp_operand_str) > MAX_OPERAND_LENGTH) {
			free(temp_operand_str);
			printf("ERROR: %s\n", error_message);
			return NULL;
		}

		int dec_value = hex_str_to_dec(temp_operand_str);
		if (dec_value < MINIMUM_MEMORY_ADDRESS_DEC || dec_value > MAXIMUM_MEMORY_ADDRESS_DEC) {
			free(temp_operand_str);
			printf("ERROR: %s\n", error_message);
			return NULL;
		}

		int required_leading_zeros = MAXIMUM_LENGTH_OF_START_OPERAND - strlen(temp_operand_str);

		for (int i = 0; i < required_leading_zeros; i++){
			prepend_char(temp_operand_str, '0');
		}
		return temp_operand_str;
	}

	else {
		free(temp_operand_str);
		printf("ERROR: Operand must be a hex memory address or a label.\n");
		return NULL;
	}
}

char* validate_operand(char* operand_str, char* opcode_str){
	if (strcmp(opcode_str, "START") == 0){
		if (validate_start_operand(operand_str) == NULL){
			printf("ERROR: Invalid START operand.\n");
			return NULL;
		}
		return operand_str;
	}
	else if(strcmp(opcode_str, "END") == 0){
		// END operand is not used. We will ignore it
		// and consider the END opcode the last
		// instruction in the assembly code program.
		return operand_str;
	}
	else if(strcmp(opcode_str, "BYTE") == 0){
		if (validate_byte_operand(operand_str) == NULL){
			printf("ERROR: Invalid BYTE operand.\n");
			return NULL;
		}
		return operand_str;
	}
	else if(strcmp(opcode_str, "WORD") == 0){
		if (validate_word_operand(operand_str) == NULL){
			printf("ERROR: Invalid WORD operand.\n");
			return NULL;
		}
		return operand_str;
	}
	else if(strcmp(opcode_str, "RESB") == 0){
		if (validate_resb_operand(operand_str) == NULL){
			return NULL;
		}
		return operand_str;
	}
	else if (strcmp(opcode_str, "RESW") == 0){
		if (validate_resw_operand(operand_str) == NULL){
			return NULL;
		}
		return operand_str;
	}
	else{
		char* validated_operand = validate_nonspecific_operand(operand_str);
		if (validated_operand == NULL){
			return NULL;
		}
		return validated_operand;
	}
}


void print_parsed_line_of_code(struct parsed_line_of_code_struct* parsed_line_of_code_struct_ptr){
	printf("Line Number           : %d\n", parsed_line_of_code_struct_ptr->line_number);
	printf("Is Comment            : %s\n", parsed_line_of_code_struct_ptr->is_comment ? "true" : "false");
	printf("Is Index Addressing   : %s\n", parsed_line_of_code_struct_ptr->is_index_addressing ? "true" : "false");
	printf("Label                 : %s\n", parsed_line_of_code_struct_ptr->label_str);
	printf("Opcode                : %s\n", parsed_line_of_code_struct_ptr->opcode_str);
	printf("Operand               : %s\n", parsed_line_of_code_struct_ptr->operand_str);
	printf("Unparsed Line of Code : %s\n\n", parsed_line_of_code_struct_ptr->unparsed_line_of_code_str);
}

// START LINKED LIST
struct node_struct{
	int index;
	struct parsed_line_of_code_struct *parsed_line_of_code_struct_ptr;
	struct node_struct *next_ptr;
	struct node_struct *previous_ptr;
};

struct node_struct *g_head_ptr = NULL;
struct node_struct *g_tail_ptr = NULL;

void add(struct parsed_line_of_code_struct *parsed_line_of_code_struct_ptr){
	if (g_head_ptr == NULL){
		g_head_ptr = (struct node_struct*) malloc(sizeof(struct node_struct));
		g_head_ptr->index = 0;
		g_head_ptr->parsed_line_of_code_struct_ptr = parsed_line_of_code_struct_ptr;
		g_head_ptr->previous_ptr = NULL;
		g_head_ptr->next_ptr = NULL;
		g_tail_ptr = g_head_ptr;
		return;
	}

	struct node_struct *new_ptr = NULL;

	new_ptr = (struct node_struct*) malloc(sizeof(struct node_struct));
	new_ptr->index = g_tail_ptr->index + 1;
	new_ptr->parsed_line_of_code_struct_ptr = parsed_line_of_code_struct_ptr;
	new_ptr->previous_ptr = g_tail_ptr;
	new_ptr->next_ptr = NULL;
	g_tail_ptr->next_ptr = new_ptr;
	g_tail_ptr = new_ptr;
}

void print_list(){
	struct node_struct *current_ptr = g_head_ptr;
    do{
    	print_parsed_line_of_code(current_ptr->parsed_line_of_code_struct_ptr);
    	current_ptr = current_ptr->next_ptr;
    } while (current_ptr != NULL);

}
// END LINKED LIST

int main()

{
	FILE *assembly_code_file;

	char unparsed_line_of_code_str[MAX_LINE_LENGTH];

	// Open input file for reading and check for existence of input file
	assembly_code_file = fopen("/Users/nickjackson/Desktop/Assembly Code/ReadWriteTest.asm", "r");
	if (assembly_code_file == NULL)						// Input file could not be opened.
	{
		printf("Input file does not exist. \n");
		return -1;
	}

	// Create parsed line of code structure
	int line_number = 0;
	struct parsed_line_of_code_struct *parsed_line_of_code_struct_ptr;
	// Print the contents of the file
	while (fgets(unparsed_line_of_code_str, MAX_LINE_LENGTH, assembly_code_file)!=NULL){
		line_number += 1;
		parsed_line_of_code_struct_ptr = create_parsed_line_of_code_struct(line_number, unparsed_line_of_code_str);
        print_parsed_line_of_code(parsed_line_of_code_struct_ptr);

        // Blank Lines indicate an error.
        if (test_for_blank_line(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str)){
        	printf("Parser Error: Line is blank. Line must contain code or comment\n LINE %d\n",
        	        parsed_line_of_code_struct_ptr->line_number);
        	return -1;
        }

        // Put line of code into the array list
        add(parsed_line_of_code_struct_ptr);

        // Check for comment line
        if (test_for_comment_line(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str)){
        	parsed_line_of_code_struct_ptr->is_comment = true;
        	continue;
        }

        // Determine if line of code has a label.
        bool has_label = test_for_label(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str);

        char* byte_character_string = get_byte_character_string(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str);

        // Tokenize the unparsed line of code
        struct split_token_array_struct* token_array_struct_ptr = split_line_of_code(parsed_line_of_code_struct_ptr->unparsed_line_of_code_str);

        // Handle spaces within the byte character and reconfigure token array when necessary
        if (byte_character_string != NULL){
        	handle_byte_character_string(token_array_struct_ptr, byte_character_string);
        }

        if (has_label && token_array_struct_ptr->length == 3){
        	if (validate_label(token_array_struct_ptr->token_array[0]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->label_str, token_array_struct_ptr->token_array[0]);

        	if (validate_opcode(token_array_struct_ptr->token_array[1]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->opcode_str, token_array_struct_ptr->token_array[1]);

        	if (strcmp(parsed_line_of_code_struct_ptr->opcode_str, "RSUB") &&
        		strcmp(parsed_line_of_code_struct_ptr->opcode_str, "XOS") &&
        		strcmp(parsed_line_of_code_struct_ptr->opcode_str, "END")){

        		char* validated_operand = validate_operand(token_array_struct_ptr->token_array[2], parsed_line_of_code_struct_ptr->opcode_str);
        		if (validated_operand == NULL){
        			return -1;
        		}
        		strcpy(parsed_line_of_code_struct_ptr->operand_str, validated_operand);
        		parsed_line_of_code_struct_ptr->is_index_addressing = g_is_indexed_addressing;
        	}
        }
        // Handles cases where remarks are not present in line of code
        else if (has_label && token_array_struct_ptr->length == 2){
        	if (validate_label(token_array_struct_ptr->token_array[0]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->label_str, token_array_struct_ptr->token_array[0]);

        	if (validate_opcode(token_array_struct_ptr->token_array[1]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->opcode_str, token_array_struct_ptr->token_array[1]);
        }
        else if ((has_label == false) && token_array_struct_ptr->length >= 2){
        	if (validate_opcode(token_array_struct_ptr->token_array[0]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->opcode_str, token_array_struct_ptr->token_array[0]);

        	if (strcmp(parsed_line_of_code_struct_ptr->opcode_str, "RSUB") &&
        		strcmp(parsed_line_of_code_struct_ptr->opcode_str, "XOS") &&
        		strcmp(parsed_line_of_code_struct_ptr->opcode_str, "END")){

        		char* validated_operand = validate_operand(token_array_struct_ptr->token_array[1], parsed_line_of_code_struct_ptr->opcode_str);
        		if (validated_operand == NULL){
        			return -1;
        		}
        		strcpy(parsed_line_of_code_struct_ptr->operand_str, validated_operand);
        		parsed_line_of_code_struct_ptr->is_index_addressing = g_is_indexed_addressing;
        	}
        }
        else if ((has_label == false) && token_array_struct_ptr->length == 1){
        	if (validate_lone_opcode(token_array_struct_ptr->token_array[0]) == NULL){
        		return -1;
        	}
        	strcpy(parsed_line_of_code_struct_ptr->opcode_str, token_array_struct_ptr->token_array[0]);
        }
        else{
        	printf("ERROR: Line of code cannot be parsed.");
        	return -1;
        }
    }


    // Output List
    print_list();

    fclose(assembly_code_file);
	return 0;
}