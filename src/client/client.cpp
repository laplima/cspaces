#include <iostream>
#include <print>
#include <format>
#include <colibry/ORBManager.h>
#include <colibry/NameServer.h>
#include <TupleSpaceC.h>
#include <stdexcept>
#include <string>
#include <colibry/LineShell.h>
#include <spdlog/spdlog.h>

using namespace std;
using namespace CSpace;

constexpr const char* const TSFACNAME = "TSFac";

istream& operator>>(istream& is, Tuple& t);
ostream& operator<<(ostream& os, const Tuple& t);

using colibry::lineshell::Stringv;

template<typename T>
string str(const T& t)
{
	ostringstream ss;
	ss << t;
	return ss.str();
}

class MyCommands : public colibry::CmdObserver {
public:
	explicit MyCommands(TupleSpaceFactory_ptr facref)
		: tsfac{TupleSpaceFactory::_duplicate(facref)}
	{
		bind()
		("getts", FWRAP(getts))
		("out", FWRAP(out))
		("in", FWRAP(in))
		("rd", FWRAP(rd))
		("inp", FWRAP(inp))
		("rdp", FWRAP(rdp));
	}
private:
	void getts(const Stringv& args);
	void out(const Stringv& args);
	void in(const Stringv& args);
	void rd(const Stringv& args);
	void inp(const Stringv& args);
	void rdp(const Stringv& args);

	TupleSpaceFactory_var tsfac = TupleSpaceFactory::_nil();
	TupleSpace_var ts = TupleSpace::_nil();
};

int main(int argc, char* argv[])
{
    const char* facname = argv[1];
    if (facname == nullptr)
		facname = TSFACNAME;
    
    try {
		print("* Initializing... ");
		colibry::ORBManager om{argc, argv};
		colibry::NameServer ns{om};
		TupleSpaceFactory_var fac = ns.resolve<TupleSpaceFactory>(facname);
		println("OK\n\tTSFactory:  {}", facname);

		MyCommands cmds{fac.in()};
		colibry::LineShell sh{cmds};
		colibry::lineshell::PersistenceManager::load_str(sh, R"([
			{
				"getts": {
					"desc": "get a (new) tuplespace",
					"args": [] 
				}
			},
			{
				"out": {
					"desc": "output tuple to TS",
					"args": []
				}
			},
			{
				"in": {
					"desc": "blocking input tuple from TS (removing it) ",
					"args": []
				}
			},
			{
				"rd": {
					"desc": "blocking input tuple from TS",
					"args": []
				}
			},
			{
				"inp": {
					"desc": "non-blocking input tuple from TS (removing it) ",
					"args": []
				}
			},
			{
				"rdp": {
					"desc": "non-blocking input tuple from TS",
					"args": []
				}
			},
			{
				"help": {
					"desc": "list all commands available",
					"args": []
				}
			},
			{
				"exit": {
					"desc": "quit the client",
					"args": []
				}
			}
		])");
	
		sh.set_prompt("> ");
		sh.cmdloop();
		
    } catch (CosNaming::NamingContext::NotFound&) {
		spdlog::error("CS Factory \"{}\" is not registered in NS.", facname);
		return 1;
    } catch (CORBA::Exception& e) {
		spdlog::error("CORBA exception [{}]", str(e));
    }
}

// Suppose tuple is <int,string,float>
istream& operator>>(istream& is, Tuple& t)
{
    // Read tuple

    string aux;
    print("\tCORBA::Short = ");
    getline(is,aux);
    if (!aux.empty())
		t[0] <<= static_cast<CORBA::Short>(stoi(aux));
    
    print("\tstring = ");
    getline(is,aux);
    if (!aux.empty())
		t[1] <<= aux.c_str();
    
    print("\tCORBA::Float = ");
    getline(is,aux);
    if (!aux.empty())
		t[2] <<= static_cast<CORBA::Float>(stof(aux));
    return is;
}

ostream& operator<<(ostream& os, const Tuple& t)
{
    os << "<";
    CORBA::ULong len = t.length();
    for (CORBA::ULong i=0; i<len; i++) {
		switch (t[i].type()->kind()) {
		case CORBA::tk_null:  // wildcard
		    os << "*";
		    break;
		case CORBA::tk_short: // Short
		    CORBA::Short x;
		    t[i] >>= x;
		    os << x;
		    break;
		case CORBA::tk_float: // Float
		    CORBA::Float y;
		    t[i] >>= y;
		    os << y;
		    break;
		case CORBA::tk_string: // String
		    const char* s;
		    t[i] >>= s;
		    os << "\"" << s << "\"";
		    break;
		default:
		    os << "unknown";
		}
		if (i < len-1) // not the last?
		    os << ",";
    }    
    os << ">";
    return os;
}

// COMMANDS

void MyCommands::getts(const Stringv& args)
{
    if (args.size() < 2)
		throw runtime_error{format("Missing parameter for {}", args[0])};

    try {
		ts = tsfac->get(args[1].c_str());
		println("\tTupleSpace: {}", args[1]);
		parentls()->set_prompt(format("{}> ", args[1]));
    } catch (CORBA::Exception& e) {
		spdlog::error("CORBA exception [{}]", str(e));
    }
}

void MyCommands::out(const Stringv& args)
{
    if (CORBA::is_nil(ts.in())) {
		spdlog::error("\tNo tuple space.");
		return;
    }
	
	try {
	    Tuple t(3);
	    t.length(3);
	    cin >> t;

	    println("\t{} {}",args[0], str(t));
	    ts->out(t);
	} catch(const CSpace::InvalidTuple& ) {
		spdlog::error("Tuple must not contain wildcards");
	}
}

void MyCommands::in(const Stringv& args)
{
    if (CORBA::is_nil(ts.in())) {
		spdlog::error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\t{} {}", args[0], str(t));
    ts->in(t);
    println("\tResult = {}", str(t));
}

void MyCommands::rd(const Stringv& args)
{
    if (CORBA::is_nil(ts.in())) {
		spdlog::error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\t{} {}", args[0], str(t));
    ts->rd(t);
    println("\tResult = {}", str(t));
}

void MyCommands::inp(const Stringv& args)
{
    if (CORBA::is_nil(ts.in())) {
		spdlog::error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\t{} {}", args[0], str(t));
    if (ts->inp(t))
    	println("\tResult = {}", str(t));
    else
		println("\tNot found");
}

void MyCommands::rdp(const Stringv& args)
{
    if (CORBA::is_nil(ts.in())) {
		spdlog::error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\t{} {}", args[0], str(t));
    if (ts->rdp(t))
	    println("\tResult = {}", str(t));
    else
		println("\tNot found");	
}
