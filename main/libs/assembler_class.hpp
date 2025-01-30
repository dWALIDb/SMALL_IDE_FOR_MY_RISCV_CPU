#ifndef __MY_ASSEMBLER
#define __MY_ASSEMBLER

// MAKE SURE  THAT YOU SET THE VALIDITY TO TRUE, ERROR MESSAGE CLEARED 
// AND PROGRAM COUNTER TO 0 BEFORE DOING ANY WORK IN PROCESSING FUNCTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <unordered_map>

#define ERROR_MESSAGE "TOKENIZER FAILED: PLEASE VERIFY CURRENT INSTRUCTION\n"

class assembler
{
private:
    int program_counter = 0;
    int jump;
    std::ofstream output_file;
    std::ifstream input_file;
    std::unordered_map<std::string, int> label_map;
    std::unordered_map<std::string, int>::iterator it;
    public:
    std::string error_message;
    bool valid_code=true;
    bool valid_generation=true;

public:
    std::string form_machine_code(std::string opcode, std::string operands)
    { // if encountered a label or comment then nothing is here to process
        if (opcode.at(0) == '.' || opcode.at(0) == ';')
        {
            return " ";
        }
        //detection of origin changes the value of PC 
        if (opcode.at(0) == '#')
        {
            opcode = strip_from_char(opcode, '#');
            this->program_counter = stoi(opcode);

            while (opcode.length() != 7)
            {
                opcode = "0" + opcode;
            }

            return "#" + opcode;
        }

        std::cout << "\n"
                  << opcode << ":" << "\n";
        //*************************/ INSTRUCTIONS WITH NO OPERANDS : <opcode>
        if (opcode == "interrupt_disable")
        {
            return "00000000000000000000000000011111";
        }
        else if (opcode == "nop") // it is encoded as add x0,x0,x0
        {
            return "00000000000000000000000000110011";
        }
        // OPERAND1 holds write back address mostly or first read address,
        // OPERAND2 holds first read address of second address depending on instruction,
        // OPERAND3 holds the second read value or offset

        std::string OPERAND1 = "", OPERAND2 = "", OPERAND3 = "", immediate_value = "";

        OPERAND1 = get_substring(operands, operands.at(0), ',');
        OPERAND3 = get_substring(operands, ',', '\n');
        // OPERAND3 at first contains both OPERAND2 and OPERAND3 to strip it we have to get lengths and OPERAND3 is always larger
        // '\n' was added to make this substring easier to extract(can't believe that i cant spaghetti code my way out xD)
        OPERAND1 = strip_from_char(OPERAND1, ',');
        OPERAND3 = strip_from_char(OPERAND3, ',');
        std::cout << "operand1: " << OPERAND1 << "\n";
        std::cout << "operand3: " << OPERAND3;

        OPERAND1 = determine_register(OPERAND1);
        //************************/ INSTRUCTIONS OF THE FORM     <OPCODE> <OPERAND1>,<OPERAND3>
        if (opcode == "jal")
        {
            it = label_map.find(OPERAND3);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                OPERAND3 = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << OPERAND3 + "\n";
            }
            OPERAND3 = convert_to_stringOF_binary(OPERAND3, 20);
            // fileds are divided this way 1 11111111 1 1111111111
            return OPERAND3.at(0) + OPERAND3.substr(10, 10) + OPERAND3.at(9) + OPERAND3.substr(1, 8) + OPERAND1 + "1101111";
        }
        else if (opcode == "auipc")
        {
            OPERAND3 = convert_to_stringOF_binary(OPERAND3, 20);
            return OPERAND3 + OPERAND1 + "0010111";
        }
        else if (opcode == "lui")
        {
            OPERAND3 = convert_to_stringOF_binary(OPERAND3, 20);
            return OPERAND3 + OPERAND1 + "0110111";
        }
        else if (opcode == "fmv.w.x") // from int to fp
        {
            OPERAND3 = determine_register(OPERAND3);
            return "111100000000" + OPERAND3 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fmv.x.w") // from fp to int
        {
            OPERAND3 = determine_register(OPERAND3);
            return "111000000000" + OPERAND3 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fcvt.w.s") // convert from fp to int(SIGNED)
        {
            OPERAND3 = determine_register(OPERAND3);
            return "110000000000" + OPERAND3 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fcvt.s.w") // convert from int to fp(SIGNED)
        {
            OPERAND3 = determine_register(OPERAND3);
            return "110100000000" + OPERAND3 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fmv") // SIGN INJECTION INSTRUCTION:THEY TAKE THE WRITE ADDRESS AND ONE REGISTER AND NOT 3 OPERANDS
        {                         // encoded as FSGNJ.S kinda a work around because im lazy
            OPERAND3 = determine_register(OPERAND3);
            return "001000000000" + OPERAND3 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fneg")
        { // encoded as FSGNJN.S kinda a work around because im lazy
            OPERAND3 = determine_register(OPERAND3);
            return "001000000000" + OPERAND3 + "001" + OPERAND1 + "1010011";
        }
        else if (opcode == "fabs")
        { // encoded as FSGNJX.S kinda a work around because im lazy
            OPERAND3 = determine_register(OPERAND3);
            return "001000000000" + OPERAND3 + "010" + OPERAND1 + "1010011";
        }
        else if (opcode == "in_data") // input data from user just like sw but from user
        {
            OPERAND3 = convert_to_stringOF_binary(OPERAND3, 12);
            return OPERAND3.substr(0, 7) + "00000" + OPERAND1 + "000" + OPERAND3.substr(7, 5) + "1110111";
        }
        else if (opcode == "out_data") // output data to the user just like lw but to the user
        {
            OPERAND3 = convert_to_stringOF_binary(OPERAND3, 12);
            return OPERAND3 + OPERAND1 + "000" + "00000" + "0001000";
        }

        OPERAND2 = get_substring(operands, ',', ',');
        OPERAND3 = get_substring(operands, ',', '\n');
        OPERAND3 = OPERAND3.substr(OPERAND2.length() - 1, OPERAND3.length() - OPERAND2.length() + 1);

        //********************/ all instruction of form: /<opcode> operand1,operand2,operand3/ are dealt with
        OPERAND2 = strip_from_char(OPERAND2, ',');
        OPERAND3 = strip_from_char(OPERAND3, ',');

        std::cout << "operand2: " << OPERAND2 << "\n";
        std::cout << "operand3: " << OPERAND3 << "\n";

        OPERAND2 = determine_register(OPERAND2);
        immediate_value = OPERAND3;

        // lots of instructions share the same opcode with different function fields
        // idk how to deal with that exept with this way xD (sry)
        if (opcode == "add") //--------REGISTER ARITHMETIC OPERATIONS
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "0110011";
        }
        else if (opcode == "sub")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0100000" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "0110011";
        }
        else if (opcode == "or")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "110" + OPERAND1 + "0110011";
        }
        else if (opcode == "and")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "111" + OPERAND1 + "0110011";
        }
        else if (opcode == "xor")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "100" + OPERAND1 + "0110011";
        }
        else if (opcode == "sll")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "001" + OPERAND1 + "0110011";
        }
        else if (opcode == "srl")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "101" + OPERAND1 + "0110011";
        }
        else if (opcode == "sra")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "101" + OPERAND1 + "0110011";
        }
        else if (opcode == "slt")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "010" + OPERAND1 + "0110011";
        }
        else if (opcode == "sltu")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "011" + OPERAND1 + "0110011";
        }
        else if (opcode == "fadd") // F EXTENSION INSTRUCTION
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000000" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fmul")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0001000" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fdiv")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0001100" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fmin")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0010100" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "1010011";
        }
        else if (opcode == "fmax")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0010100" + OPERAND3 + OPERAND2 + "001" + OPERAND1 + "1010011";
        }
        else if (opcode == "flw") // LOAD WORD
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "010" + OPERAND1 + "0000111";
        }
        else if (opcode == "fsw") // STORE WORD
        {                         // OPERAND1 has value to store , OPERAND2 and immediate value has the offset
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value.substr(0, 7) + OPERAND2 + OPERAND1 + "010" + immediate_value.substr(7, 5) + "0100111";
        }
        else if (opcode == "mul") //----M EXTENSION INSTRUCTION
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "000" + OPERAND1 + "0110011";
        }
        else if (opcode == "mulh")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "001" + OPERAND1 + "0110011";
        }
        else if (opcode == "mulhsu")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "010" + OPERAND1 + "0110011";
        }
        else if (opcode == "mulhu")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "011" + OPERAND1 + "0110011";
        }
        else if (opcode == "div")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "100" + OPERAND1 + "0110011";
        }
        else if (opcode == "divu")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "101" + OPERAND1 + "0110011";
        }
        else if (opcode == "rem")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "110" + OPERAND1 + "0110011";
        }
        else if (opcode == "remu")
        {
            OPERAND3 = determine_register(OPERAND3);
            return "0000001" + OPERAND3 + OPERAND2 + "111" + OPERAND1 + "0110011";
        }
        else if (opcode == "addi") //-----REGISTER IMMEDIATE VALUE OPERANDS
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "000" + OPERAND1 + "0010011";
        }
        else if (opcode == "slti")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "010" + OPERAND1 + "0010011";
        }
        else if (opcode == "sltiu")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "011" + OPERAND1 + "0010011";
        }
        else if (opcode == "xori")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "100" + OPERAND1 + "0010011";
        }
        else if (opcode == "ori")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "110" + OPERAND1 + "0010011";
        }
        else if (opcode == "andi")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "111" + OPERAND1 + "0010011";
        }
        else if (opcode == "slli")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 5);
            return "0000000" + immediate_value + OPERAND2 + "001" + OPERAND1 + "0010011";
        }
        else if (opcode == "srli")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 5);
            return "0000000" + immediate_value + OPERAND2 + "101" + OPERAND1 + "0010011";
        }
        else if (opcode == "srai")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 5);
            return "0100000" + immediate_value + OPERAND2 + "101" + OPERAND1 + "0010011";
        }
        else if (opcode == "beq") // CONDITIONAL BRANCHES
        {                         // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }

            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "000" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "bneq")
        { // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "001" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "blt")
        { // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "100" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "bge")
        { // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "101" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "bltu")
        { // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "110" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "bgeu")
        { // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
          // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            immediate_value = convert_to_stringOF_binary(immediate_value, 11);
            // strings are indexed from left to right this is why msb is at index "0" of string :)
            return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + "111" + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
        }
        else if (opcode == "jalr") // JUMP AND LINK REGISTER
        {
            // Because of the way i wrote the tokenization OPERAND1 is first operand and OPERAND2 is second operand
            // OPERAND3 has the offset and we take the same amount but we take 11 bits and concatinate 0 at the lsb
            it = label_map.find(immediate_value);
            if (it != label_map.end())
            {

                this->jump = 0;
                this->jump = it->second - this->program_counter - 4;
                jump = jump / 4;
                immediate_value = std::to_string(this->jump);
                std::cout << "*labeled offset found* " << "\n";
                std::cout << "offset: " << immediate_value + "\n";
            }
            // OPERAND1 stores PC+4 and OPERAND2 and immediate_value has the offset
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "000" + OPERAND1 + "1100111";
        }
        else if (opcode == "lw") // LOAD WORD
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "010" + OPERAND1 + "0000011";
        }
        else if (opcode == "sw") // STORE WORD
        {                        // OPERAND1 has value to store , OPERAND2 and immediate value has the offset
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);

            return immediate_value.substr(0, 7) + OPERAND2 + OPERAND1 + "010" + immediate_value.substr(7, 5) + "0100011";
        }
        else if (opcode == "interrupt_enable")
        {
            immediate_value = convert_to_stringOF_binary(immediate_value, 12);
            return immediate_value + OPERAND2 + "000" + OPERAND1 + "0111111";
        }

        // if the instruction is undefined then we get an error message
        return ERROR_MESSAGE;
    }

    void assemble(std::string input_path, std::string output_path)
    {
        this->valid_code=true;
        this->error_message.clear();
        this->program_counter = 0;
        std::string instruction = "", opcode = "", operands = "", label = "", machine_code = "";

        std::cout << "\n***************** RISC V ASSEMBLER *********************** \n";

        if (input_file.is_open())
        {
            input_file.close();
        }
        if (output_file.is_open())
        {
            output_file.close();
        }
        
        input_file.open(input_path);
        
        if (input_file.is_open())
        {
            std::cout << "\n***************** GETTING LABELS *********************** \n";
            while (!input_file.eof())
            {
                std::getline(input_file, label);
                if (label=="")
                {
                    label="; \n";
                }

                label = strip_from_char(label, ' ');

                get_labels(label);
            }
        }
        else
        {
            std::cout<<"\nCAN'T OPEN FILE at "+input_path+"\n";
            this->error_message="CAN'T OPEN FILE AT "+input_path;
            this->valid_code=false;
        }
        
        input_file.close();

        this->program_counter = 0;

        input_file.open(input_path);
        output_file.open(output_path);

        if (input_file.is_open() && output_file.is_open())
        {
            std::cout << "\n***************** ASSEMBLING *********************** \n";
            std::cout << "\nfile openned successfully \n\n";
            while (!input_file.eof())
            {
                // we get the opcode and the operands
                input_file >> opcode;
                // opcode=strip_from_char(opcode,' ');

                std::getline(input_file, operands); // getline omits '\n' so we add it to get
                // strip from white spaces and comments
                if (operands.find(';') != std::string::npos)
                {
                    operands = get_substring(operands, operands.at(0), ';');
                    operands = strip_from_char(operands, ';');
                }
                operands = strip_from_char(operands, ' ');

                opcode = to_lowercase(opcode);
                // operands=to_lowercase(operands);

                // little spaghetti code to bypass some problems when getting substrings for last arguments (we all love a good pasta):)
                operands = operands + "\n";
                // check if the extracted instruction is viable
                machine_code = form_machine_code(opcode, operands);
                // ------------------------------------
                // check if there is an invalid string
                // ------------------------------------

                if (machine_code==ERROR_MESSAGE)
                {
                    std::cout<<"\nINSTRUCTION: "+ opcode+" "+operands+" AT LINE: "+std::to_string(this->program_counter/4+1)+" IS NOT VALID\n";
                    this->error_message="INSTRUCTION: "+ opcode+" "+operands+" AT LINE: "+std::to_string(this->program_counter/4+1)+" IS NOT VALID";
                    this->valid_code=false;
                    break;
                }
                

                std::cout << "line :" << this->program_counter/4+1 << " instruction: " << machine_code;

                if (machine_code != " " && machine_code.at(0) != '#')
                { // strings index from left to right so msb of instruction is encoded at most left(or index 0)
                    output_file << machine_code.substr(24, 8) << '\n';
                    output_file << machine_code.substr(16, 8) << '\n';
                    output_file << machine_code.substr(8, 8) << '\n';
                    output_file << machine_code.substr(0, 8) << '\n';
                }
                else if (machine_code.at(0) == '#')
                {
                    output_file << machine_code << '\n';
                }
                // no increment when we have comment/label or change of address origin
                if (machine_code != " " && machine_code.at(0) != '#')
                {
                    program_counter += 4;
                }
            }
        }
        else{
            std::cout << "\n failed to open text files, please check file paths \n";
            std::cout<<"CAN'T OPEN FILES at "+input_path+" and "+output_path;
            this->error_message="CAN'T OPEN FILES AT "+input_path+"\nAND "+output_path;
            this->valid_code=false;
        }
        std::cout << "\n***************** THANKS FOR VISITING *********************** \n";
        input_file.close();
        output_file.close();
        // this->program_counter = 0;
    }

    std::string strip_from_char(std::string input, char char_to_strip)
    {
        std::string output;
        for (int i = 0; i < input.size(); i++)
        {
            // input.at() acts as input[i] i just learnt it xD
            if (input.at(i) != char_to_strip)
            {
                output += input.at(i);
            }
        }
        return output;
    }

    std::string get_substring(std::string input, char start, char end)
    {
        // find first occurence of a char for star and end indexes then return the substring
        // get the substring including start and end
        std::string output;
        int start_of_substring = input.find(start), end_of_substring;

        if (start_of_substring != std::string::npos)
        {
            end_of_substring = input.find(end, start_of_substring + 1);
        }

        if (start_of_substring == std::string::npos || end_of_substring == std::string::npos)
        {
            return ERROR_MESSAGE;
        }
        output = input.substr(start_of_substring, end_of_substring - start_of_substring + 1);
        return output;
    }
    // register have the synthax of x<register number> so this function considers that
    std::string determine_register(std::string input)
    {
        std::string output = "";
        input = strip_from_char(input, 'x');

        int value = stoi(input, 0, 10);
        // convert to binary
        int binary[5] = {0};
        // we need 5 bits for the field of register
        for (int i = 0; i < 5; i++)
        {

            binary[i] = value % 2;

            value = value / 2;
            //    convert to binary and write to a string
            if (binary[i] == 1)
            {
                output = "1" + output;
            }
            else
                output = "0" + output;
        }

        return output;
    }

    std::string convert_to_stringOF_binary(std::string input, int number_of_bits)
    {
        std::string output = "";

        int value = stoi(input, 0, 10);
        // conver to 2's complement
        if (value < 0)
        {
            value = pow(2, number_of_bits) + value; // calculate 2's complement /2^N-value/ but since value<0 so we add directly
        }
        int binary[number_of_bits] = {0};
        // we need 5 bits for the field of register
        for (int i = 0; i < number_of_bits; i++)
        {

            binary[i] = value % 2;

            value = value / 2;
            //    convert to binary and write to a string
            if (binary[i] == 1)
            {
                output = "1" + output;
            }
            else
                output = "0" + output;
        }

        return output;
    }

    void get_labels(std::string input)
    {
        if (input.at(0) == '.')
        {
            input = strip_from_char(input, '.');
            label_map.insert({input + '\n', program_counter});
            this->label_map[input] = program_counter;
            std::cout << "label: " << input << " at " << this->label_map[input] << "\n";
        }
        else if (input.at(0) == '#')
        {
            input = strip_from_char(input, '#');
            this->program_counter = stoi(input);
        }
        else if (input.at(0) != ';')
        {
            this->program_counter += 4;
        }
    }

    void generate_byte_mif(std::string binary_file_path, std::string mif_file_path, int depth = 32)
    {
        input_file.open(binary_file_path);
        output_file.open(mif_file_path);
        std::string value = "";
        if (input_file.is_open() && output_file.is_open())
        {
            this->program_counter = 0;
            std::cout << "\n FILE OPPENED SUCCESSFULLY \n";
            std::cout << "\n **************** GENERATING MIF ****************  \n";
            output_file << "DEPTH= " << depth << ";\n";
            output_file << "WIDTH= 8" << ";\n";
            output_file << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file << "CONTENT BEGIN" << "\n";
            output_file << "[0.." << depth - 1 << "]:" << "00000000;\n";
            while (!input_file.eof() && std::getline(input_file, value))
            {
                if (value.at(0) == '#')
                {
                    value = strip_from_char(value, '#');
                    this->program_counter = stoi(value);
                }
                else if (value != "" && value != "  " && value != " \n")
                {
                    output_file << this->program_counter << " : " << value << ";\n";
                    program_counter += 1;
                }

                if (this->program_counter > depth)
                {
                    std::cout << "\nDATA CONTENT EXCEEDES THE MIF SIZE QUITTING...\n";
                    return;
                }
            }
            output_file << "END;\n";
        }
        else
            std::cout << "\n failed to open text files, please check file paths \n";

        std::cout << "\n\n THANKS FOR VISITING \n\n";

        this->program_counter = 0;

        input_file.close();
        output_file.close();
    }

    std::string to_lowercase(std::string input)
    {
        std::string output = "";
        for (int i = 0; i < input.length(); i++)
        {
            // chars can be treated as integers :) so 'A' is 65 and 'a' is 97 difference is 32
            if (input.at(i) >= 'A' && input.at(i) <= 'Z')
            {
                input.at(i) = input.at(i) + 32;
            }
            output += input.at(i);
        }
        return output;
    }

    void generate_bytesFor_32bits(std::string binary_file_path, std::string mif_0path,std::string mif_1path,std::string mif_2path,std::string mif_3path, int depth=64)
    {
        this->valid_generation=true;
        this->program_counter = 0;
        this->error_message.clear();
        input_file.open(binary_file_path);
        std::ofstream output_file0;
        std::ofstream output_file1;
        std::ofstream output_file2;
        std::ofstream output_file3;
        output_file0.open(mif_0path);
        output_file1.open(mif_1path);
        output_file2.open(mif_2path);
        output_file3.open(mif_3path);

        std::string value = "";
        if (input_file.is_open())
        {
            std::cout<<"input file is open\n";
        }
        if (output_file0.is_open())
        {
            std::cout<<"output file0 is open\n";
        }
        if (output_file1.is_open())
        {
            std::cout<<"output file1 is open\n";
        }
        if (output_file2.is_open())
        {
            std::cout<<"output file2 is open\n";
        }
        if (output_file3.is_open())
        {
            std::cout<<"output file3 is open\n";
        }
        
        if (input_file.is_open() && output_file0.is_open() && output_file1.is_open() && output_file2.is_open() && output_file3.is_open())
        {
            std::cout << "\n FILES OPPENED SUCCESSFULLY \n";
            std::cout << "\n **************** GENERATING MIF ****************  \n";
            output_file0 << "DEPTH= " << depth << ";\n";
            output_file0 << "WIDTH= 8" << ";\n";
            output_file0 << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file0 << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file0 << "CONTENT BEGIN" << "\n";
            output_file0 << "[0.." << depth - 1 << "]:" << "00000000;\n";

            output_file1 << "DEPTH= " << depth << ";\n";
            output_file1 << "WIDTH= 8" << ";\n";
            output_file1 << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file1 << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file1 << "CONTENT BEGIN" << "\n";
            output_file1 << "[0.." << depth - 1 << "]:" << "00000000;\n";

            output_file2 << "DEPTH= " << depth << ";\n";
            output_file2 << "WIDTH= 8" << ";\n";
            output_file2 << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file2 << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file2 << "CONTENT BEGIN" << "\n";
            output_file2 << "[0.." << depth - 1 << "]:" << "00000000;\n";

            output_file3 << "DEPTH= " << depth << ";\n";
            output_file3 << "WIDTH= 8" << ";\n";
            output_file3 << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file3 << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file3 << "CONTENT BEGIN" << "\n";
            output_file3 << "[0.." << depth - 1 << "]:" << "00000000;\n";
            
            while (!input_file.eof() && std::getline(input_file, value))
            {
                if (value.at(0) == '#')
                {
                    value = strip_from_char(value, '#');
                    this->program_counter = stoi(value);
                }
                else if (value != "" && value != "  " && value != " \n")
                {
                    output_file0 << this->program_counter << " : " << value << ";\n";
                    std::getline(input_file, value);
                    output_file1 << this->program_counter << " : " << value << ";\n";
                    std::getline(input_file, value);
                    output_file2 << this->program_counter << " : " << value << ";\n";
                    std::getline(input_file, value);
                    output_file3 << this->program_counter << " : " << value << ";\n";
                    this->program_counter += 1;
                }

                if (this->program_counter > depth)
                {
                    this->valid_generation=false;
                    this->error_message="CAN'T FIT THE DATA IN THE DESIRED SIZE";
                    std::cout << "\nDATA CONTENT EXCEEDES THE MIF SIZE QUITTING...\n";
                    break;
                }
            }
            output_file0 << "END;\n";
            output_file1 << "END;\n";
            output_file2 << "END;\n";
            output_file3 << "END;\n";
        }
        else{
            std::cout << "\n failed to open text files, please check file paths \n";
            this->error_message="FAILED TO OPEN MIF FILES";
            this->valid_generation=false;
        }

        std::cout << "\n\n THANKS FOR VISITING \n\n";

        // this->program_counter = 0;

        input_file.close();
        output_file0.close();
        output_file1.close();
        output_file2.close();
        output_file3.close();
        
    }

int Get_program_counter(){return this->program_counter;}
};

#endif
