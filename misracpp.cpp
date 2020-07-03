//===- PrintFunctionNames.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include <clang/AST/Decl.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Serialization/PCHContainerOperations.h>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

#include <sstream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <utility>
#include <cctype>
using namespace std;


namespace {

clang::LangOptions lopt;

std::string decl2str(SourceManager *sm, clang::Decl *d) {
    clang::SourceLocation b(d->getBeginLoc()), _e(d->getEndLoc());
    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, *sm, lopt));
    auto ret = std::string(sm->getCharacterData(b), sm->getCharacterData(e)-sm->getCharacterData(b));
    for (int i = 0;i < ret.length(); i++) if (ret[i] == '\n') ret[i] = ' ';
    return ret;
    
}
class MyVisitor : public RecursiveASTVisitor<MyVisitor> {
public:
    explicit MyVisitor(ASTContext* Context) : Context(Context) {}
    bool VisitFunctionDecl(FunctionDecl* FD) {
        /*if (FD->getFirstDecl()->isStatic() && !FD->isStatic()) {
            FD->dump();
        }*/

        auto &SourceManager = Context->getSourceManager();

        stringstream ss;
        string functionName = FD->getNameAsString();
        size_t paramSize = FD->param_size();
        string returnArg = FD->getReturnType().getAsString();
        
        ss << "{ \"name\": \"" << functionName << "\",";
        ss << "  \"return_type\": \"" << returnArg << "\",";
        ss << "  \"parameters\": [";
        vector<pair<string, string>> params;
        for(size_t i = 0; i < paramSize; i++){
            ParmVarDecl* param = FD->getParamDecl(i);
            auto type = param->getType().getAsString();
            auto fullparam = decl2str(&SourceManager, param);
            auto name = param->getNameAsString();
            ss << "{ \"full\": \"" << fullparam << "\", ";
            ss << "  \"analysed\": [\"" << type << "\", \"" << name << "\"] } ";
            params.push_back(make_pair(type, name));

            if(i < paramSize - 1) ss << ", ";
        }
        ss << "] }";

        // ss << "cudnnStatus_t CUDNNWINAPI " << functionName << "(";

        // vector<pair<string, string>> params;
        // for(size_t i = 0; i < paramSize; i++){
        //     ParmVarDecl* param = FD->getParamDecl(i);
        //     auto type = param->getType().getAsString();
        //     auto name = param->getNameAsString();
        //     ss << type << " " << name;
        //     params.push_back(make_pair(type, name));

        //     if(i < paramSize - 1) ss << ", ";
        // }
        // ss << ")"; 
        

        // ss << "{\n";
        // ss << "\t" << "PRINT(\"Enter " << functionName << "\\n\");\n";                                             
        // ss << "\t" << "typedef decltype(&" << functionName << ") funcType;\n";    
        // ss << "\t" << "funcType func = (funcType) actualDlsym(libcudnnHandle, STRINGIFY(" << functionName << "));\n";    
        // ss << "\t" << "fromCudnnApiName(STRINGIFY(" << functionName << "));\n";
        
        // ss << "\n";

        // for(size_t i = 0; i < params.size(); i++){
        //     auto the = params[i];
        //     string typeName = "";
        //     if(the.first == "const void *"){
        //         typeName = "Input";
        //     }else if (the.first == "void *"){
        //         typeName = "Output";
        //     } 

        //     //llvm::outs() << "-\t" << the.second  << " "  << typeName << "\n";
        //     if (typeName.length() > 0){
        //         for(size_t j = 0; j < params.size(); j++){
        //             auto rel = params[j];
        //             int declLoc = rel.first.find("Descriptor_t");
        //             int cudnnLoc = rel.first.find("cudnn");

        //             if(i != j && declLoc != string::npos){
        //                 size_t camelLoc = 0;
        //                 while(camelLoc < rel.second.length() && !isupper(rel.second[camelLoc])) camelLoc++;
        //                 string firstCamel = rel.second.substr(0, camelLoc);
        //                 //llvm::outs() << "-\t" << firstCamel << "\n"; 

        //                 size_t compare = 0;
        //                 while(compare < min(firstCamel.length(), the.second.length()) &&
        //                       firstCamel[compare] == the.second[compare]) compare++;
        //                 if(compare == firstCamel.length()){
        //                     size_t declTypeLen = declLoc - (cudnnLoc + 5);
        //                     string declTypeName = rel.first.substr(cudnnLoc + 5, declTypeLen);
        //                     ss << "\t" << "fromCudnn" << typeName << declTypeName << "(" << rel.second << ", " << the.second << ");\n";
        //                 }
        //             }
        //         }
        //     }
        // }

        // ss << "\n";

        // ss << "\t" << "cudnnStatus_t ret = func(";
        // for(size_t i = 0; i < params.size(); i++){
        //     auto the = params[i];
        //     ss << the.second;
        //     if(i < paramSize - 1) ss << ", ";
        // }
        // ss << ");\n";
        // ss << "\treturn ret;\n";
        
        // ss << "}\n";


        llvm::outs() << ss.str() << "\n";
        return true;
    }
    bool VisitVarDecl(VarDecl* VD) {
        //llvm::outs() << VD->getNameAsString() << " " << declToString(VD->getFirstDecl()) << " " << declToString(VD) << "\n";
        return true;
    }
    std::string declToString(DeclaratorDecl* DD) {
        // return DD->getType().getAsString();
        return Lexer::getSourceText(CharSourceRange::getTokenRange(DD->getTypeSourceInfo()->getTypeLoc().getSourceRange()),
            Context->getSourceManager(), Context->getLangOpts());
    }

private:
    ASTContext* Context;
};  // namespace

class PrintFunctionsConsumer : public ASTConsumer {
public:
    explicit PrintFunctionsConsumer(CompilerInstance& CI) : CI(CI) {}
    virtual void HandleTranslationUnit(clang::ASTContext& Context) {
        // Traversing the translation unit decl via a RecursiveASTVisitor
        // will visit all nodes in the AST.
        MyVisitor(&Context).TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    CompilerInstance& CI;
    // A RecursiveASTVisitor implementation.
};

class PrintFunctionNamesAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<PrintFunctionsConsumer>(CI);
    }

    bool ParseArgs(const CompilerInstance& CI, const std::vector<std::string>& args) override {
        return true;
    }
    void PrintHelp(llvm::raw_ostream& ros) {
        ros << "Help for PrintFunctionNames plugin goes here\n";
    }
};

}  // namespace

static FrontendPluginRegistry::Add<PrintFunctionNamesAction> X("print-fns", "print function names");
