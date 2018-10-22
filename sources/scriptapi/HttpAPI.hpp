
#include <lua.h>
#include <lauxlib.h>
#include <httplib.h>
#include "Logger.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/circular_buffer.hpp>

namespace Lua
{
	using namespace httplib;
	using namespace boost;
	using namespace boost::lockfree;

	typedef struct HttpServer_Obj 
	{
		Server *svr;
	}HttpServer_Obj;

#define HTTP_LIB_SERVER "HTTPLIB_SERVER"
#define to_HttpServer_Obj(L) ((HttpServer_Obj*)luaL_checkudata(L, 1, HTTP_LIB_SERVER))

	struct HttpRequestEvent
	{
		const httplib::Request* req;
		bool ready = false;
		std::condition_variable* cv;
		std::mutex* mutex;
	};

	inline static spsc_queue<HttpRequestEvent, capacity<10>, fixed_sized<true>> s_http_req_evts;

	static HttpServer_Obj *check_HttpServer_Obj(lua_State *L)
	{
		HttpServer_Obj *o = to_HttpServer_Obj(L);
		if (o->svr == nullptr)
			luaL_error(L, "attempt to use an invalid or closed object");
		return o;
	}

	static int httplib_create(lua_State *L)
	{
		int n = lua_gettop(L);  /* number of arguments */
		if (n != 2)
		{
			spdlog::error("{}->wrong number of arguments", __FUNCTION__);
			return 0;
		}
		auto host = lua_tostring(L, -2);
		auto port = lua_tointeger(L, -1);

		HttpServer_Obj* o = static_cast<HttpServer_Obj*>(lua_newuserdata(L, sizeof(HttpServer_Obj)));
		o->svr = new httplib::Server();
		o->svr->listen(host, port);
		luaL_getmetatable(L,  HTTP_LIB_SERVER);
		lua_setmetatable(L, -2);

		return 1;
	}

	static int httplib_get_route(lua_State *L)
	{
		return 0;
	}

	static int httplib_post_route(lua_State *L)
	{
		HttpServer_Obj *o = check_HttpServer_Obj(L);
		int lua_callback = luaL_ref(L, LUA_REGISTRYINDEX);
		o->svr->Post("/*", [=](const Request& req, Response& res)
		{
			HttpRequestEvent event;
			event.cv = new std::condition_variable();
			event.mutex = new std::mutex();
			if (s_http_req_evts.write_available() > 0)
			{
				s_http_req_evts.push(event);
				std::unique_lock<std::mutex> lk(*event.mutex);
				event.req = &req;
				event.cv->wait(lk, [&]{return event.ready;});
			}
			res.set_content(lua_tostring(L, -1), "text/plain");
		});
		return 0;
	}

	static void http_event_loop(lua_State* L)
	{
		if (s_http_req_evts.read_available() > 0)
		{
			HttpRequestEvent evt;
			while (s_http_req_evts.pop(evt))
			{
				int lua_callback = 0;
				lua_rawgeti(L, LUA_REGISTRYINDEX, lua_callback);
				int n = evt.req->params.size();
				lua_pushstring(L, evt.req->path.c_str());
				lua_newtable(L);
				for (auto[k, v] : evt.req->params)
				{
					lua_pushstring(L, k.c_str());
					lua_pushstring(L, v.c_str());
					lua_settable(L, -3);
				}
				if (lua_pcall(L, 2, 1, 0) != LUA_OK)
				{
					spdlog::error(lua_tostring(L, -1));
				}
			}
		}
	}

	static const struct luaL_Reg httplibapi_func_list[] = {
		{"create", httplib_create},
		{"get", httplib_get_route},
		{"post", httplib_post_route},
		{NULL, NULL},
	};

	LUALIB_API int httplib_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		luaL_newlib(L, httplibapi_func_list);
		return 1;
	}

	/*----------------------------------------------------------------------
	 * main entry function; library registration
	 *----------------------------------------------------------------------
	 */

	int httplib_register(lua_State *L)
	{
		/* enum metatable */
		hidapi_create_hidenum_obj(L);
		/* device handle metatable */
		hidapi_create_hiddevice_obj(L);
		/* library */
		luaL_requiref(L, "hidapi", hid_functions_bind, 0);

		lua_pushliteral(L, "_VERSION");
		lua_pushliteral(L, MODULE_VERSION);
		lua_settable(L, -3);

		lua_pushliteral(L, "_TIMESTAMP");
		lua_pushliteral(L, MODULE_TIMESTAMP);
		lua_settable(L, -3);

		return 1;
	}
}
