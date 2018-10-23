local M = {}
local httpapi = require("httpapi")
local server = nil
local handlers = {POST = {}, GET = {}}

function M.init(host, port)
	server = httpapi.create(host, port)
	server:route(M.route)
end

function M.route(method, path, params)
	return handlers[method][path](params)
end

function M.get(path, callback)
	handlers.GET[path] = callback
end

function M.post(path, callback)
	handlers.GET[path] = callback
end

return M
