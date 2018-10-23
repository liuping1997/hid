
#include <lua.h>
#include <lauxlib.h>
#include <httplib.h>
#include "Logger.hpp"
#include <boost/lockfree/spsc_queue.hpp>

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
		int lua_callback = -1;
		std::shared_ptr< std::condition_variable> cv;
		std::shared_ptr<std::mutex> m;
		std::string context;
	};

	inline static spsc_queue<HttpRequestEvent*, capacity<10>, fixed_sized<true>> s_http_req_evts;

	static HttpServer_Obj *check_HttpServer_Obj(lua_State *L)
	{
		HttpServer_Obj *o = to_HttpServer_Obj(L);
		if (o->svr == nullptr)
			luaL_error(L, "attempt to use an invalid or closed object");
		return o;
	}

	static int http_create(lua_State *L)
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
		std::thread* t = new std::thread([=]()
		{
			o->svr->listen(host, port);
		});
		luaL_getmetatable(L,  HTTP_LIB_SERVER);
		lua_setmetatable(L, -2);

		return 1;
	}

	static int http_route(lua_State *L)
	{
		HttpServer_Obj *o = check_HttpServer_Obj(L);
		int lua_callback = luaL_ref(L, LUA_REGISTRYINDEX);
		auto callback = [=](const Request& req, Response& res)
		{
			if (s_http_req_evts.write_available() == 0)
			{
				spdlog::error("route error.sq is not eough.");
				return;
			}
			HttpRequestEvent evt;
			evt.cv = std::make_shared<std::condition_variable>();
			evt.m = std::make_shared<std::mutex>();
			std::unique_lock<std::mutex> lk(*evt.m);
			evt.req = &req;
			evt.lua_callback = lua_callback;
			s_http_req_evts.push(&evt);
			evt.cv->wait(lk, [&] {return evt.ready; });
			lk.unlock();
			evt.cv->notify_one();
			res.set_content(evt.context, "text/plain");
		};
		o->svr->Get("/(\\w*/)*(\\w*)*/?", callback);
		o->svr->Post("/(\\w*/)*(\\w*)*/?", callback);
		return 0;
	}

	static void http_event_loop(lua_State* L)
	{
		if (s_http_req_evts.read_available() == 0)
		{
			return;
		}

		HttpRequestEvent* evt_ptr;
		while (s_http_req_evts.pop(evt_ptr))
		{
			HttpRequestEvent& evt = *evt_ptr;
			lua_rawgeti(L, LUA_REGISTRYINDEX, evt.lua_callback);
			lua_pushstring(L, evt.req->method.c_str());
			lua_pushstring(L, evt.req->path.c_str());
			lua_newtable(L);
			for (auto[k, v] : evt.req->params)
			{
				lua_pushstring(L, k.c_str());
				lua_pushstring(L, v.c_str());
				lua_settable(L, -3);
			}
			{
				std::lock_guard<std::mutex> lk(*(evt.m));
				evt.ready = true;
			}
			if (lua_pcall(L, 3, 1, 0) != LUA_OK)
			{
				spdlog::error(lua_tostring(L, -1));
			}
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				evt.context = luaL_checkstring(L, -1);
			}
			else if (lua_type(L, -1) == LUA_TNUMBER)
			{
				evt.context = std::to_string(luaL_checknumber(L, -1));
			}
			else
			{
				evt.context = "return error";
			}
			evt.cv->notify_one();
		}
	}
	
	static int http_server_meta_gc(lua_State *L)
	{
		HttpServer_Obj *o = to_HttpServer_Obj(L);
		if (o->svr) 
		{
			o->svr->stop();
			delete o->svr;
			o->svr = NULL;
		}
		return 0;
	}

	static const struct luaL_Reg http_server_meta_reg[] = {
		{"route", http_route},
		{"__gc", http_server_meta_gc},
		{NULL, NULL},
	};

	static const struct luaL_Reg http_func_list[] = {
			{"create", http_create},
			{NULL, NULL},
	};

	static void http_create_server_obj(lua_State *L) {
		luaL_newmetatable(L, HTTP_LIB_SERVER);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, http_server_meta_reg, 0);
	}

	LUALIB_API int http_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		luaL_newlib(L, http_func_list);
		return 1;
	}
	/*----------------------------------------------------------------------
	 * main entry function; library registration
	 *----------------------------------------------------------------------
	 */
	int http_register(lua_State *L)
	{
		/* http server handle metatable */
		http_create_server_obj(L);
		/* library */
		luaL_requiref(L, "httpapi", http_functions_bind, 0);
		return 1;
	}
}
