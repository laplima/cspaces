05/06/2007
REQUIREMENTS:

- NamingService running

SERVER OBJECTS (in tsfac)

- TupleSpaceFactory: generates new tuple spaces or returns existing
ones.
- TupleSpace: receives Linda calls (in, rd, out, inp, rdp).

LIMITATIONS:

- Server threadpool limited to 10 threads. If 10 clients are blocked
waiting for in or rd operations to complete, the server is not able
anymore to receive out calls (i.e. all 10 threads are blocked).
- IsEqual method should deal with ALL data types that may be contained
in CORBA::Any data type. Currently, it's dealing with all basic data
types:

	- CORBA::Short
	- CORBA::Long
	- CORBA::UShort
	- CORBA::ULong
	- CORBA::Float
	- CORBA::Double
	- const char* (strings)
	- CORBA::Boolean
	- CORBA::Octet
	- CORBA::Char

18/02/2026
The project has been modernized, but still needs testing. Blocking rd, for example, is not working.
Client has not been updated (only csclient).
