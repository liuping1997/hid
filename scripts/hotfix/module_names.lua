-- Module names need hotfix.
-- hotfix_helper.lua will reload this module in check().
-- So it can be changed dynamically.

local hotfix_module_names = {
  "main",
  "hid",
  "http",
}

return hotfix_module_names
