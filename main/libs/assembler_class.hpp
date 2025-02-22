#ifndef __MY_ASSEMBLER
#define __MY_ASSEMBLER

// MAKE SURE  THAT YOU SET THE VALIDITY TO TRUE, ERROR MESSAGE CLEARED 
// AND PROGRAM COUNTER TO 0 BEFORE DOING ANY WORK IN PROCESSING FUNCTIONS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>//for pow and other stuff
#include <sstream>//to convert strings into streams and tokenizing with getline
#include <unordered_map>//to store labels and instruction properties
#include <tuple>//to store opcodes instruction format funct3/7 in same place 

#define ERROR_MESSAGE "TOKENIZER FAILED: PLEASE VERIFY CURRENT INSTRUCTION\n"
/*
EACH LINE HAS ONE FUNCTIONALITY
AN INSTRUCTION
A LABLE (STARTING WITH .)
A PC REDIRECT (STARINT WITH #)
COMMENTS ARE STARTED BY ;
TO USE HEX VALUES YOU NEED $BEFORE THE HEX IMMEDIATE
DATA CAN BE STORED IN PROGRAM BUT ANOTHER FUNCTION PROCESSES THEM TO GENERATE DATA FILE
/name: for the data words they are separated with ,
2 TYPES OF FILES ,DATA FILE AND INSTRUCTION FILE

LABLES ARE STORED WITH PC INCREMENTING WITH 4 ON EACH INSTRUCTION
THIS MEANS THAT WHEN DETECTING A LABLE FOR BRANCHES THE ASSEMBLER WILL DEVIDE THAT ADDRESS BY 4
SO IN BRANCHES JUST COUNT THE INSTRUCTIONS TO SKIP AND USE THAT COUNT AS THE OFFSET DIRECTLY
*/
class assembler
{
private:
// this is used to map each instruction of riscv format to a std_unordered map
// R format has funct 3 and funt 7
// I S B formats have funct 3 and immediate values so the are handeled differently
// U and J formats have immediate values directly 
// the unordered map has now a mnemonic  that is mapped to all the properties it needes 
// it is as follows mnemonic -> opcode, format , funct3 , funct7
const std::unordered_map<std::string,std::tuple<std::string,unsigned char,std::string,std::string>> 
    //mnemonic opcode type funct3 funct7
instruction_properties={
    //*************************/ INSTRUCTIONS WITH NO OPERANDS : <opcode> 
    {"interrupt_disable",std::tuple("0011111",'R',"00000000","00000000000000000")},
    {"nop",std::tuple("0110011",'R',"00000000","00000000000000000")},        
    //************************/ INSTRUCTIONS OF THE FORM     <OPCODE> <OPERAND1>,<OPERAND3>
    {"jal",std::tuple("1101111",'J',"/","/")},
    {"aupic",std::tuple("0010111",'U',"/","/")},
    {"lui",std::tuple("0110111",'U',"/","/")},
    {"fmv.w.x",std::tuple("1010011",'R',"000","111100000000")},//add 5 zeroes before funct 7 because we have no operand2 and no rounding module so funct3 is 0 :)
    {"fmv.x.w",std::tuple("1010011",'R',"000","111000000000")},
    {"fcvt.w.s",std::tuple("1010011",'R',"000","110000000000")},
    {"fcvt.s.w",std::tuple("1010011",'R',"000","110100000000")},
    {"fmv",std::tuple("1010011",'R',"000","001000000000")},//encoded as FSGNJ.S SIGN INJECTION INSTRUCTION:THEY TAKE THE WRITE ADDRESS AND ONE REGISTER AND NOT 3 OPERANDS
    {"fneg",std::tuple("1010011",'R',"001","001000000000")},//encoded as FSGNJN.S SIGN INJECTION INSTRUCTION:THEY TAKE THE WRITE ADDRESS AND ONE REGISTER AND NOT 3 OPERANDS
    {"fabs",std::tuple("1010011",'R',"010","001000000000")},//encoded as FSGNJX.S SIGN INJECTION INSTRUCTION:THEY TAKE THE WRITE ADDRESS AND ONE REGISTER AND NOT 3 OPERANDS
    {"in_data",std::tuple("1110111",'S',"000","/")},// input data from user just like sw but from user source and immediate
    //for in data use 00000 after operand 1 decoded alone
    //************************/ INSTRUCTIONS OF THE FORM     <OPCODE> <OPERAND1>,<OPERAND2>,<OPERAND3>
    {"out_data",std::tuple("0001000",'I',"00000000","/")},// output data to the user just like lw but to the user
    {"add",std::tuple("0110011",'R',"000","0000000")},
    {"sub",std::tuple("0110011",'R',"000","0100000")},
    {"or",std::tuple("0110011",'R',"110","0000000")},
    {"and",std::tuple("0110011",'R',"111","0000000")},
    {"xor",std::tuple("0110011",'R',"100","0000000")},
    {"sll",std::tuple("0110011",'R',"001","0000000")},
    {"sra",std::tuple("0110011",'R',"101","0100000")},
    {"srl",std::tuple("0110011",'R',"101","0000000")},
    {"slt",std::tuple("0110011",'R',"010","0000000")},
    {"sltu",std::tuple("0110011",'R',"011","0000000")},
    {"fadd",std::tuple("1010011",'R',"000","0000000")},//F extension
    {"fmul",std::tuple("1010011",'R',"000","0001000")},
    {"fdiv",std::tuple("1010011",'R',"000","0001100")},
    {"fmin",std::tuple("1010011",'R',"000","0010100")},
    {"fmax",std::tuple("1010011",'R',"001","0010100")},
    {"flw",std::tuple("0000111",'I',"010","/")},
    {"fsw",std::tuple("0100111",'S',"010","/")},
    {"mul",std::tuple("0110011",'R',"000","0000001")},//----M EXTENSION INSTRUCTION
    {"mulh",std::tuple("0110011",'R',"001","0000001")},
    {"mulhsu",std::tuple("0110011",'R',"010","0000001")},
    {"mulhu",std::tuple("0110011",'R',"011","0000001")},
    {"div",std::tuple("0110011",'R',"100","0000001")},
    {"divu",std::tuple("0110011",'R',"101","0000001")},
    {"remu",std::tuple("0110011",'R',"111","0000001")},
    {"addi",std::tuple("0010011",'I',"000","/")},//-----REGISTER IMMEDIATE VALUE OPERANDS
    {"slti",std::tuple("0010011",'I',"010","/")},
    {"sltiu",std::tuple("0010011",'I',"011","/")},
    {"xori",std::tuple("0010011",'I',"100","/")},
    {"ori",std::tuple("0010011",'I',"110","/")},
    {"andi",std::tuple("0010011",'I',"111","/")},
    {"slli",std::tuple("0010011",'I',"001","0000000")},
    {"srli",std::tuple("0010011",'I',"101","0000000")},
    {"srai",std::tuple("0010011",'I',"101","0100000")},
    {"beq",std::tuple("1100011",'B',"000","/")},// CONDITIONAL BRANCHES 
    {"bneq",std::tuple("1100011",'B',"001","/")}, 
    {"blt",std::tuple("1100011",'B',"100","/")}, 
    {"bge",std::tuple("1100011",'B',"101","/")}, 
    {"bltu",std::tuple("1100011",'B',"110","/")}, 
    {"bgeu",std::tuple("1100011",'B',"111","/")}, 
    {"jalr",std::tuple("1100111",'I',"000","/")}, 
    {"lw",std::tuple("0000011",'I',"010","/")}, 
    {"sw",std::tuple("0100011",'S',"010","/")}, 
    {"interrupt_enable",std::tuple("0111111",'I',"000","/")},  
    };

    unsigned int program_counter = 0;
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
    //generates series of 32 bits corresponding to the instruction
    //lables and data words are not detected with this instruction
    std::string form_machine_code(std::string opcode, std::string operands)
    { // if encountered a label or comment then nothing is here to process
        if (opcode.at(0) == '.' || opcode.at(0)=='/' || opcode.at(0) == ';')
        {
            return " ";
        }
        //detection of origin changes the value of PC 
        if (opcode.at(0) == '#')
        {
            opcode = strip_from_char(opcode, '#');
            //detect if data is in hex or not
            if(opcode.find('$')==std::string::npos){
                this->program_counter = stoull(opcode);
                return "#" + opcode;
            }
            else
            {
                opcode = strip_from_char(opcode, '$');
                this->program_counter = stoull(opcode,0,16);
            }
            // while (opcode.length() != 7)
            // {
            //     opcode = "0" + opcode;
            // }

            return "#" + opcode+'$';
        }

        std::cout << "\n"
                  << opcode << ":" << "\n";
        
        // OPERAND1 holds write back address mostly or first read address,
        // OPERAND2 holds first read address of second address depending on instruction,
        // OPERAND3 holds the second read value or offset

        std::string OPERAND1 = "??", OPERAND2 = "??", OPERAND3 = "??", immediate_value = "??";
        // detect if it is an instruction that has no/two/three operands
        int first_coma=operands.find(',');
        if (first_coma!= std::string::npos && opcode!="nop" && opcode!="interrupt_disable")
        {
            OPERAND1 = get_substring(operands, operands.at(0), ',');
            OPERAND3 = get_substring(operands, ',', '\n');
            // OPERAND3 at first contains both OPERAND2 and OPERAND3 to strip it we have to get lengths and OPERAND3 is always larger
            // '\n' was added to make this substring easier to extract(can't believe that i cant spaghetti code my way out xD)
            OPERAND1 = strip_from_char(OPERAND1, ',');
            OPERAND3 = strip_from_char(OPERAND3, ',');
            std::cout << "operand1: " << OPERAND1 << "\n";
            OPERAND1 = determine_register(OPERAND1);
            int second_coma=operands.find_last_of(',');
            // determine if the instruction has 2 operands only
            if (second_coma!= std::string::npos && first_coma!=second_coma)
            {
                
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

            }else
            {
                std::cout << "operand3: " << OPERAND3 << "\n";
            }
            
        }
        try
        {   
            //test the instruction format to determine the overall structure of instruction
            switch (std::get<1>(instruction_properties.at(opcode)))
            {
            case 'I':
                    //immediate instructions
                    //shift instructions are special cases
                    //$ used to indicate that the number is in hex  
                    if (immediate_value.find('$')!=std::string::npos)
                    {
                        immediate_value=strip_from_char(immediate_value,'$');
                        immediate_value=std::to_string(stoull(immediate_value,0,16));
                    }
                    if (opcode=="slli" || opcode=="srli" || opcode=="srai")
                    {
                        immediate_value = convert_to_stringOF_binary(immediate_value, 5);
                        return std::get<3>(instruction_properties.at(opcode))+immediate_value+OPERAND2 + std::get<2>(instruction_properties.at(opcode)) + OPERAND1 +std::get<0>(instruction_properties.at(opcode));    
                    }
                    //labeled values give the address directly
                    //this means that addi x1,x0,label will give the address of label in x1 
                    it = label_map.find(immediate_value);
                    if (it != label_map.end())
                    { 
                        immediate_value =std::to_string(it->second);
                        std::cout << "*labeled value found* " << "\n";
                        std::cout << "value: " << immediate_value + "\n";
                    }
                    if (opcode=="out_data")
                    {
                        if (OPERAND3.find('$')!=std::string::npos)
                        {
                            OPERAND3=strip_from_char(OPERAND3,'$');
                            OPERAND3=std::to_string(stoull(OPERAND3,0,16));
                        }
                        OPERAND3 = convert_to_stringOF_binary(OPERAND3, 12);
                        return OPERAND3 + OPERAND1 + "000" + "00000" + "0001000";
                    }
                    immediate_value = convert_to_stringOF_binary(immediate_value, 12);
                    return immediate_value + OPERAND2 + std::get<2>(instruction_properties.at(opcode)) + OPERAND1 + std::get<0>(instruction_properties.at(opcode)) ;
                break;
            
            case 'R':
                    if (opcode == "nop" || opcode == "interrupt_disable")
                    {
                        OPERAND3="";OPERAND1="";
                        return std::get<3>(instruction_properties.at(opcode)) + OPERAND3 + std::get<2>(instruction_properties.at(opcode)) + OPERAND1 + std::get<0>(instruction_properties.at(opcode));
                    }
                    OPERAND3 = determine_register(OPERAND3);
                    if (opcode == "fmv.w.x" || opcode == "fmv.x.w" || opcode == "fcvt.w.s" || opcode == "fcvt.s.w" || opcode == "fmv" || opcode == "fneg" || opcode == "fabs")
                    {
                        OPERAND2="";
                        // return std::get<3>(instruction_properties.at(opcode)) + OPERAND3 + std::get<2>(instruction_properties.at(opcode)) + OPERAND1 + std::get<0>(instruction_properties.at(opcode));
                    }
                    return std::get<3>(instruction_properties.at(opcode)) + OPERAND3 + OPERAND2 + std::get<2>(instruction_properties.at(opcode)) + OPERAND1 + std::get<0>(instruction_properties.at(opcode));
                break;
            
            case 'B':
                    //branch instructions
                    //$ used to indicate that the number is in hex  
                    if (immediate_value.find('$')!=std::string::npos)
                    {
                        immediate_value=strip_from_char(immediate_value,'$');
                        immediate_value=std::to_string(stoull(immediate_value,0,16));
                    }
                    it = label_map.find(immediate_value);
                    if (it != label_map.end())
                    {
                        // determine the offset to jump to
                        this->jump = 0;
                        //offsets are calculated when pc is incremented so this has tobe kept in mind
                        this->jump = it->second - this->program_counter - 4;
                        jump = jump / 4;
                        immediate_value = std::to_string(this->jump);
                        std::cout << "*labeled offset found* " << "\n";
                        std::cout << "offset: " << immediate_value + "\n";
                    }
                    immediate_value = convert_to_stringOF_binary(immediate_value, 11);
                    // strings are indexed from left to right this is why msb is at index "0" of string :)
                    return immediate_value.at(0) + immediate_value.substr(2, 6) + OPERAND2 + OPERAND1 + std::get<2>(instruction_properties.at(opcode)) + immediate_value.substr(8, 3) + "0" + immediate_value.at(1) + "1100011";
                break;

            case 'S':
                //mostly store instructions 
                //$ used to indicate that the number is in hex  
                    if (immediate_value.find('$')!=std::string::npos)
                    {
                        immediate_value=strip_from_char(immediate_value,'$');
                        immediate_value=std::to_string(stoull(immediate_value,0,16));
                    }
                    it = label_map.find(immediate_value);
                    if (it != label_map.end())
                    { 
                        immediate_value =std::to_string(it->second);
                        std::cout << "*labeled value found* " << "\n";
                        std::cout << "value: " << immediate_value + "\n";
                    }
                    if (opcode=="in_data")
                    {
                        //$ used to indicate that the number is in hex  
                        if (OPERAND3.find('$')!=std::string::npos)
                        {
                            OPERAND3=strip_from_char(OPERAND3,'$');
                            OPERAND3=std::to_string(stoull(OPERAND3,0,16));
                        }
                        OPERAND3 = convert_to_stringOF_binary(OPERAND3, 12);
                        return OPERAND3.substr(0, 7) + "00000" + OPERAND1 + std::get<2>(instruction_properties.at(opcode)) + OPERAND3.substr(7, 5) + std::get<0>(instruction_properties.at(opcode)) ;
                    }
                    immediate_value = convert_to_stringOF_binary(immediate_value, 12);
                    return immediate_value.substr(0, 7) + OPERAND2 + OPERAND1 + std::get<2>(instruction_properties.at(opcode)) + immediate_value.substr(7, 5) + std::get<0>(instruction_properties.at(opcode));
                break;

            case 'U':
            //instructions that use large immediates
            //$ used to indicate that the number is in hex  
                    if (OPERAND3.find('$')!=std::string::npos)
                    {
                        OPERAND3=strip_from_char(OPERAND3,'$');
                        OPERAND3=std::to_string(stoull(OPERAND3,0,16));
                    }
                    it = label_map.find(OPERAND3);
                    if (it != label_map.end())
                    { 
                        OPERAND3 =std::to_string(it->second);
                        std::cout << "*labeled value found* " << "\n";
                        std::cout << "value: " << OPERAND3 + "\n";
                    }
                    // get all 32 bits then take 20 upper ones (strings start from 0 on the left where MSB will be given in this function)
                    // no need for %hi() and %lo() to get the data give 32 bits and it handles the rest
                    OPERAND3 = convert_to_stringOF_binary(OPERAND3, 32);
                    return OPERAND3.substr(12,31) + OPERAND1 + std::get<0>(instruction_properties.at(opcode));
                break;

            case 'J':
            //instructions that use large immdiates with special bit ordering
            //$ used to indicate that the number is in hex  
                if (OPERAND3.find('$')!=std::string::npos)
                {
                    OPERAND3=strip_from_char(OPERAND3,'$');
                    OPERAND3=std::to_string(stoull(OPERAND3,0,16));
                }
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
                    return OPERAND3.at(0) + OPERAND3.substr(10, 10) + OPERAND3.at(9) + OPERAND3.substr(1, 8) + OPERAND1 + std::get<0>(instruction_properties.at(opcode)) ;
                break;
            default:
                break;
            }
            
        }
        //when getting values from tuples some instructions might not be found because of some error when user writes code
        // thus it gives an exeption we catch it and then we process it as instruction not found
        catch(const std::exception& e)
        {
            std::cerr << ERROR_MESSAGE << '\n';
            std::cerr << e.what() << '\n';
        }

        return ERROR_MESSAGE;
    }
    // the most important instruction
    // it takes an input file (containing the instructions)
    // the output file will be used to dump all the instructions as bits
    // they are organized as 8 bits in each line 0-7 then next line 8-15...
    // why? cuz it might be used to generate one large byte addressable memory
    // or 4 word addressable memories (some flexebility)
    void assemble(std::string input_path, std::string output_path)
    {
        this->valid_code=true;
        this->error_message.clear();
        this->program_counter = 0;
        std::string instruction = "", opcode = "", operands = "", label = "", machine_code = "";

        std::cout << "\n***************** RISC V ASSEMBLER *********************** \n";
        //if used with win32 api gotta close them 
        //else the program crashes here for some reason
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
                //affects program counter
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
                 //each line takes 8 bits starting from lsb
                    output_file << machine_code.substr(24, 8) << '\n';
                    output_file << machine_code.substr(16, 8) << '\n';
                    output_file << machine_code.substr(8, 8) << '\n';
                    output_file << machine_code.substr(0, 8) << '\n';
                }
                else if (machine_code.at(0) == '#')
                {
                    output_file << machine_code << '\n';
                }
                else if(opcode.at(0)=='/')
                {
                    output_file << opcode+operands;
                }
                // no increment when we have comment/label or change of address origin
                if (machine_code != " " && machine_code.at(0) != '#')
                {
                    this->program_counter += 4;
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
    //pretty helpfull to process strings with special delimiters
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
    //used to get some tokens of the original string
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

        int value = stoull(input, 0, 10);
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

        unsigned int value = stoull(input, 0, 10);
        // conver to 2's complement
        if (value < 0)
        {
            //this is much faster than pow()
            value = (1<<number_of_bits) + value; // calculate 2's complement /2^N-value/ but since value<0 so we add directly
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
    // processes lables as .name
    // processes data words as /name:
    void get_labels(std::string input)
    {
        if (input.at(0) == '.')
        {
            input = strip_from_char(input, '.');
            label_map.insert({input + '\n', program_counter});
            this->label_map[input] = program_counter;
            std::cout << "label: " << input << " at " << this->label_map[input] << "\n";
        }
        else if(input.at(0)=='/'){
            input = strip_from_char(input, '/');
            input=input.substr(0,input.find(':'));
            label_map.insert({input + '\n', program_counter});
            this->label_map[input] = program_counter;
            std::cout << "data words: " << input << " at " << this->label_map[input] << "\n";
        }
        else if (input.at(0) == '#')
        {
            input = strip_from_char(input, '#');
            if(input.find('$')==std::string::npos){
                this->program_counter = stoull(input);
            }
            else
            {
                input = strip_from_char(input, '$');
                this->program_counter = stoull(input,0,16);
            }
        }
        else if (input.at(0) != ';')
        {
            this->program_counter += 4;
        }
    }
    //generates the instructions for a byte addressable memory from output file of assemble() in .mif format
    void generate_instruction_8bits(std::string binary_file_path, std::string mif_file_path, int depth = 32)
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
                    if(value.find('$')==std::string::npos){
                        this->program_counter = stoull(value);
                    }
                    else
                    {
                        value = strip_from_char(value, '$');
                        this->program_counter = stoull(value,0,16);
                    }    
                }
                else if (value != "" && value != "  " && value != " \n" && value.at(0)!='/')
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
    //generates instructions for 4 byte addressable memories with .mif format
    //used for word addressable memories
    void generate_instruction_32bits(std::string binary_file_path, std::string mif_0path,std::string mif_1path,std::string mif_2path,std::string mif_3path, int depth=64)
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
                    if(value.find('$')==std::string::npos){
                        this->program_counter = stoull(value);
                    }
                    else
                    {
                        value = strip_from_char(value, '$');
                        this->program_counter = stoull(value,0,16);
                    }
                }
                else if (value != "" && value != "  " && value != " \n" && value.at(0)!='/')
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
    //generates 4 data file in .mif format generates only for /data:
    //values for data can be up to 32 bits  
    //used for programs that generate data inside the code
    //like some special strings that programer might want to use for debugging
    //could be even for other usage it is done this way because there are 2 memories instruction memory and data memory in riscv arch 
    void generate_data_32bits(std::string binary_file_path, std::string mif_0path,std::string mif_1path,std::string mif_2path,std::string mif_3path, int depth=64){
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
            //instructions are bytes in eachline
            //thus each line is a fouth of an instruction
            static float actual_count=this->program_counter;
            while (!input_file.eof() && std::getline(input_file, value))
            {
                if (value.at(0) == '#')
                {
                    value = strip_from_char(value, '#');
                    if(value.find('$')==std::string::npos){
                        this->program_counter = stoull(value);
                        actual_count=this->program_counter;
                    }
                    else
                    {
                        value = strip_from_char(value, '$');
                        this->program_counter = stoull(value,0,16);
                        actual_count=this->program_counter;

                    }
                }
                else if(value.at(0)=='/'){
                    std::string word;
                    value=strip_from_char(value,'/');
                    value=value.substr(value.find(':')+1);
                    std::cout<<"value"<<value<<std::endl;
                    int equivalent_value;
                    std::stringstream a=std::stringstream(value);
                    while (std::getline(a,word,','))
                    {
                        std::cout<<"word "<<word<<std::endl;
                        if(word.find('$')!=std::string::npos){
                            word = strip_from_char(word, '$');
                            std::cout<<"word "<<word<<std::endl;
                            word = std::to_string(std::stoull(word,0,16));
                        }   
                        word=convert_to_stringOF_binary(word,32);
                        output_file0 << this->program_counter << " : " << word.substr(24,8) << ";\n";
                        output_file1 << this->program_counter << " : " << word.substr(16,8) << ";\n";
                        output_file2 << this->program_counter << " : " << word.substr(8,8) << ";\n";
                        output_file3 << this->program_counter << " : " << word.substr(0,8) << ";\n";
                        this->program_counter += 1;
                        
                    }
                    
                }
                else if(value != "" && value != "  " && value != " \n")
                {
                    actual_count+=0.25;
                    this->program_counter=int(actual_count);
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
    //generates 1 data file in .mif format generates only for /data:
    //eventhough the memory is byte addressable the values for data can be upto 32 bits 
    //but the values are stored in bytes
    //they can be used for programs that generate data inside the code
    //like some special strings that programer might want to use for debugging
    //could be even for other usage it is done this way because there are 2 memories instruction memory and data memory in riscv arch 
    void generate_data_8bits(std::string binary_file_path, std::string mif_0path,int depth=64){
        this->valid_generation=true;
        this->program_counter = 0;
        this->error_message.clear();
        input_file.open(binary_file_path);
        std::ofstream output_file0;
        output_file0.open(mif_0path);

        std::string value = "";
        if (input_file.is_open())
        {
            std::cout<<"input file is open\n";
        }
        if (output_file0.is_open())
        {
            std::cout<<"output file0 is open\n";
        }
        
        if (input_file.is_open() && output_file0.is_open())
        {
            std::cout << "\n FILES OPPENED SUCCESSFULLY \n";
            std::cout << "\n **************** GENERATING MIF ****************  \n";
            output_file0 << "DEPTH= " << depth << ";\n";
            output_file0 << "WIDTH= 8" << ";\n";
            output_file0 << "ADDRESS_RADIX= UNS" << ";\n";//unsigned decimal data radix
            output_file0 << "DATA_RADIX= BIN" << ";\n";//binary data in each instruction
            output_file0 << "CONTENT BEGIN" << "\n";
            output_file0 << "[0.." << depth - 1 << "]:" << "00000000;\n";

            //instructions are bytes in eachline
            //thus each line is a fouth of an instruction
            static float actual_count=this->program_counter;
            while (!input_file.eof() && std::getline(input_file, value))
            {
                if (value.at(0) == '#')
                {
                    value = strip_from_char(value, '#');
                    if(value.find('$')==std::string::npos){
                        this->program_counter = stoull(value);
                        actual_count=this->program_counter;
                    }
                    else
                    {
                        value = strip_from_char(value, '$');
                        this->program_counter = stoull(value,0,16);
                        actual_count=this->program_counter;

                    }
                }
                else if(value.at(0)=='/'){
                    std::string word;
                    value=strip_from_char(value,'/');
                    value=value.substr(value.find(':')+1);
                    std::cout<<"value"<<value<<std::endl;
                    int equivalent_value;
                    std::stringstream a=std::stringstream(value);
                    while (std::getline(a,word,','))
                    {
                        std::cout<<"word "<<word<<std::endl;
                        if(word.find('$')!=std::string::npos){
                            word = strip_from_char(word, '$');
                            std::cout<<"word "<<word<<std::endl;
                            word = std::to_string(std::stoull(word,0,16));
                        }   
                        word=convert_to_stringOF_binary(word,32);
                        output_file0 << this->program_counter << " : " << word.substr(24,8) << ";\n";
                        this->program_counter += 1;
                        output_file0 << this->program_counter << " : " << word.substr(16,8) << ";\n";
                        this->program_counter += 1;
                        output_file0 << this->program_counter << " : " << word.substr(8,8) << ";\n";
                        this->program_counter += 1;
                        output_file0 << this->program_counter << " : " << word.substr(0,8) << ";\n";
                        this->program_counter += 1;
                        
                    }
                    
                }
                else if(value != "" && value != "  " && value != " \n")
                {
                    //because each instruction takes 4 lines (4 bytes)
                    this->program_counter+=1;
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
    }


    int Get_program_counter(){return this->program_counter;}
};

#endif
