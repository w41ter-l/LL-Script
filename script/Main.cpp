#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>

#include "driver.h"
#include "lexer.h"
#include "Parser.h"
#include "IRModule.h"
#include "DumpIR.h"
#include "CodeGen.h"
#include "dumpOpcode.h"
#include "OpcodeModule.h"
#include "UnreachableBlockElimination.h"
#include "CompilerInstance.h"

using namespace script;

int main(int argc, char* argv[])
{
    CompilerInstance compiler;
    auto &diag = compiler.getDiagnosisConsumer();
    diag.setMode(true);

    Driver &driver = compiler.getDriver();
    if (!driver.parseArguments(argc, argv))
        return 0;

    IRModule module;
    Lexer lexer(diag);
    Parser parser(lexer, module, diag);

    try {
        lexer.setProgram(std::string(driver.filename));

        parser.parse();
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 0;
    }

    int error = diag.errors(), warning = diag.warnings();
    std::cout << "error(" << error << "), warning(" << warning << ")" << std::endl;
    if (error)
    {
        getchar();
        return 0;
    }

#ifdef _DEBUG
	if (driver.dumpIR_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpIR dumpIR(filename += ".op_before.ir");
		dumpIR.dump(&module);
	}
#endif // _DEBUG

	if (driver.optimized_)
	{
		UnreachableBlockElimination UBElim;
		for (auto &func : module)
		{
			std::cout << "Eliminate unreachable block: " 
				<< func.first << std::endl;
			UBElim.runOnFunction(func.second);
		}
	}

	if (driver.dumpIR_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpIR dumpIR(filename += ".ir");
		dumpIR.dump(&module);
	}

    OpcodeModule opcode; 
	CodeGen codegen(opcode);
	for (auto &func : module)
	{
		std::cout << "Code generator: "
			<< func.first << std::endl;
		codegen.runOnFunction(func.second);
	}

	if (driver.dumpOpcode_)
	{
		std::string filename = driver.filename;
		filename.resize(filename.find_last_of('.'));
		DumpOpcode dumpOpcode(opcode, filename += ".opcode");
		dumpOpcode.dump();
	}

    // generate opcode
    //script::OpcodeContext opcode;
    //script::CodeGenerator codegen(opcode);
    //codegen.gen(module);

    //int length = 0;
    //Byte *opcodes = opcode.getOpcodes(length);

    //try {
    //    script::VirtualMachine vm;
    //    //buildin::BuildIn::getInstance().bindGC(vm.getGC());
    //    vm.excute(opcodes, length);
    //}
    //catch (std::runtime_error &e) {
    //    std::cout << e.what() << std::endl;
    //    return 0;
    //}

    getchar();
	return 0;
}