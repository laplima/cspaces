# CORBA TupleSpaces (CSpaces)

An experimental implementation of distributed Linda's primitives using CORBA (C++) – Jun 05th, 2007.

### LIBRARY DEPENDENCIES:

* ACE/TAO CORBA
* colibry (LineShell, ORBManager, NameServer)
* spdlog
* (requires C++23)

### RUNTIME REQUIREMENTS:

- CORBA's NamingService must be running.

### SERVER OBJECTS (in tsfac):

- `TupleSpaceFactory`: generates new tuple spaces or returns existing ones
- `TupleSpace`: receives Linda calls (`in`, `rd`, `out`, `inp`, `rdp`) (see docs for detailed explanation)

### LIMITATIONS:

- Server threadpool currently limited to 10 threads. If 10 clients are blocked waiting for `in` or `rd` operations to complete, the server is no longer able to receive out calls (i.e., all 10 threads are blocked).
- `IsEqual` method should deal with ALL data types that may be contained in a `CORBA::Any` data type. Currently, it's dealing with all basic data types:
	- `CORBA::Short`
	- `CORBA::Long`
	- `CORBA::UShort`
	- `CORBA::ULong`
	- `CORBA::Float`
	- `CORBA::Double`
	- `const char* (strings)`
	- `CORBA::Boolean`
	- `CORBA::Octet`
	- `CORBA::Char`

### Version Notes

Feb. 12th, 2026. The project has been updated to C++23 with some performance improvements.
