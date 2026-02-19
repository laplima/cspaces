#include <print>
#include <iostream>
#include <colibry/ORBManager.h>
#include <colibry/NameServer.h>
#include <TupleSpaceC.h>
#include <string>
#include <spdlog/spdlog.h>

using namespace std;
using namespace CSpace;
using namespace colibry;
using spdlog::error;

constexpr const char * const TSFACNAME = "TSFac";
constexpr const char * const TSNAME = "TS1";

TupleSpaceFactory_var tsfac = TupleSpaceFactory::_nil();
TupleSpace_var        ts    = TupleSpace::_nil();

static string gprompt;

istream& operator>>(istream& is, Tuple& t);
ostream& operator<<(ostream& os, const Tuple& t);

template<typename T>
string str(const T& t)
{
	ostringstream ss;
	ss << t;
	return ss.str();
}

void getts();
void out();
void in();
void rd();
void inp();
void rdp();

int main(int argc, char* argv[])
{
    const char* facname = argv[1];
    if (facname == nullptr)
		facname = TSFACNAME;
    
    try {
		print("* Initializing... ");
		ORBManager om{argc, argv};
		NameServer ns{om};

		tsfac = ns.resolve<TupleSpaceFactory>(facname);	
		if (CORBA::is_nil(tsfac.in())) {
	    	error("CS Factory \"{}\" is not registered in NS.",facname);
	    	return 1;
		}

		println("OK\n\tTSFactory:  {}", facname);

		gprompt = facname;

		string cmd;
		do {
	    	print("{}> ", gprompt);
	    	getline(cin,cmd);
	    	if (cmd.empty()) continue;
			if (cmd == "getts") {
				getts();
			} else if (cmd == "out") {
				out();
			} else if (cmd == "in") {
				in();
			} else if (cmd == "rd") {
				rd();
			} else if (cmd == "inp") {
				inp();
			} else if (cmd == "rdp") {
				rdp();
			} else if (cmd == "exit") {
				// nothing
			} else {
				error("Unknown command");
			}
		} while (cmd != "exit");
		
    } catch (const CosNaming::NamingContext::NotFound&) {
		error("CS Factory \"{}\" is not registered in NS.", facname);
    } catch (const CORBA::Exception& e) {
    	ostringstream ss;
    	ss << e;
		error("CORBA exception [{}]", ss.str());
    }
    return 0;
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

void getts()
{
	string tsname;
	getline(cin,tsname);
	ts = tsfac->get(tsname.c_str());
	println("\tTupleSpace: {}", tsname);
	gprompt = tsname; 
}

void out()
{
    if (CORBA::is_nil(ts.in())) {
		error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\tout {}", str(t));
    ts->out(t);
}

void in()
{
    if (CORBA::is_nil(ts.in())) {
		error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\tin {}", str(t));
    ts->in(t);
    println("\tResult = {}", str(t));
}

void rd()
{
    if (CORBA::is_nil(ts.in())) {
		error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\trd {}", str(t));
    ts->rd(t);
    println("\tResult = {}", str(t));
}

void inp()
{
    if (CORBA::is_nil(ts.in())) {
		error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\tinp {}",str(t));
    if (ts->inp(t))
		println("\tResult = {}", str(t));
    else
		println("\tNot found");
}

void rdp()
{
    if (CORBA::is_nil(ts.in())) {
		error("\tNo tuple space.");
		return;
    }
	
    Tuple t(3);
    t.length(3);
    cin >> t;

    println("\trdp {}", str(t));
    if (ts->rdp(t))
		println("\tResult = {}", str(t));
    else
		println("\tNot found");	
}

