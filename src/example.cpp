
#include<cxxopts.hpp>
#include<iostream>
#include<vector>
using namespace std;

void parse(int argc,const char* argv[]){

    try{
        cxxopts::Options options(argv[0],"My test example");
        options.allow_unrecognised_options();
        options.show_positional_help();
        options.set_width(70).set_tab_expansion().add_options("MyOption")
                ("f,float","a float variable",cxxopts::value<float>()->implicit_value("10"),"file")
                ("i,int","a integer variable",cxxopts::value<int>())
                ("l,float_list","a float list",cxxopts::value<vector<float>>())
                ("help","print help");

        auto result = options.parse(argc,argv);

        if(result.count("help")){
            cout<<options.help({"MyOption"})<<endl;
            exit(0);
        }
        if(result.count("i")){
            cout<<"Saw the argument of int "<<result.count("i")<<" times"<<endl;
            cout<<"The last value is "<<result["i"].as<int>()<<endl;
        }
        if(result.count("f")){
            cout<<"Saw the argument of float "<<result.count("f")<<" times"<<endl;
            cout<<"The last value is "<<result["f"].as<float>()<<endl;
        }
        if(result.count("l")){
            auto list = result["l"].as<vector<float>>();
            for(auto data:list){
                cout<<data<<" ";
            }
            cout<<endl;
        }
        auto unmatched = result.unmatched();
        for(auto data:unmatched){
            cout<<data<<" ";
        }
        cout<<endl;
    }catch (cxxopts::OptionException &e){
        e.what();
    }
}

int main(){
    int argc = 11;
    const char* argv[] = {"My_Group","-int","10","-i=30","20","-f","30","-help","40","-l","30,20,30,40,50,60"};
    parse(argc,argv);
    return 0;
}


