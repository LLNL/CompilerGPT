// source: https://www.boost.org/doc/libs/1_76_0/libs/json/doc/html/json/examples.html

/*
    This example parses a JSON file and pretty-prints
    it to standard output.
*/

#include <iomanip>
#include <iostream>
#include <fstream>

#include <boost/json/src.hpp>

namespace json = boost::json;

json::value
readJsonFile(std::istream& is)
{
  // adapted from the boost json documentation:
  //   https://www.boost.org/doc/libs/1_85_0/libs/json/doc/html/json/input_output.html#json.input_output.parsing

  boost::system::error_code  ec;
  boost::json::stream_parser p;
  std::string                line;

  while (std::getline(is, line))
  {
    p.write(line, ec);

    if (ec)
    {
      std::cerr << ec << std::endl;
      throw std::runtime_error("unable to parse JSON file: " + line);
    }
  }


  p.finish(ec);

  std::cerr << ec << std::endl;

  if (ec) throw std::runtime_error("unable to finish parsing JSON file");

  return p.release();
}


json::value
parse_file( char const* filename )
{
    std::ifstream f(filename);

    return readJsonFile(f);
}

std::size_t
printUnescaped(std::ostream& os, std::string_view code)
{
  std::size_t linecnt = 1;
  char        last = ' ';
  bool        lastIsLineBreak = false;

  // print to os while handling escaped characters
  for (char ch : code)
  {
    lastIsLineBreak = false;

    if (last == '\\')
    {
      switch (ch)
      {
        case 'f':  /* form feed */
                   ++linecnt;
                   os << '\n';
                   [[fallthrough]];

        case 'n':  ++linecnt;
                   os << '\n';
                   lastIsLineBreak = true;
                   break;

        case 't':  os << "  ";
                   break;

        case 'a':  /* bell */
        case 'v':  /* vertical tab */
        case 'r':  /* carriage return */
                   break;

        case '\'':
        case '"' :
        case '?' :
        case '\\': os << ch;
                   break;

        default:   os << last << ch;
      }

      last = ' ';
    }
    else if (ch == '\n')
    {
      os << ch;
      ++linecnt;
      lastIsLineBreak = true;
    }
    else if (ch == '\\')
      last = ch;
    else
      os << ch;
  }

  if (!lastIsLineBreak) os << '\n';

  return linecnt;
}


void
pretty_print( std::ostream& os, json::value const& jv, std::string* indent = nullptr )
{
    std::string indent_;
    if(! indent)
        indent = &indent_;
    switch(jv.kind())
    {
    case json::kind::object:
    {
        os << "{\n";
        indent->append(4, ' ');
        auto const& obj = jv.get_object();
        if(! obj.empty())
        {
            auto it = obj.begin();
            for(;;)
            {
                os << *indent << json::serialize(it->key()) << " : ";
                pretty_print(os, it->value(), indent);
                if(++it == obj.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "}";
        break;
    }

    case json::kind::array:
    {
        os << "[\n";
        indent->append(4, ' ');
        auto const& arr = jv.get_array();
        if(! arr.empty())
        {
            auto it = arr.begin();
            for(;;)
            {
                os << *indent;
                pretty_print( os, *it, indent);
                if(++it == arr.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "]";
        break;
    }

    case json::kind::string:
    {
        printUnescaped(os, json::serialize(jv.get_string()));
        break;
    }

    case json::kind::uint64:
        os << jv.get_uint64();
        break;

    case json::kind::int64:
        os << jv.get_int64();
        break;

    case json::kind::double_:
        os << jv.get_double();
        break;

    case json::kind::bool_:
        if(jv.get_bool())
            os << "true";
        else
            os << "false";
        break;

    case json::kind::null:
        os << "null";
        break;
    }

    if(indent->empty())
        os << "\n";
}

int
main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr <<
            "Usage: pretty <filename>"
            << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        // Parse the file as JSON
        auto const jv = parse_file( argv[1] );

        // Now pretty-print the value
        pretty_print(std::cout, jv);
    }
    catch(std::exception const& e)
    {
        std::cerr <<
            "Caught exception: "
            << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
