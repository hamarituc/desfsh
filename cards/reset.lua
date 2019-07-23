STATIC =
{
  AES(),
  DES(),
}

KEYDIV =
{
  AES("00000000000000000000000000000000"),
  AES("11111111111111111111111111111111"),
}

debugset(255)
cmd.select(0)


authkey = nil

for _, k in ipairs(STATIC) do
  result, msg = cmd.auth(0, k)

  if result == 0 then
    authkey = k
    break
  end
end

if authkey == nil then
  _, _, v = cmd.getver()

  for _, mk in ipairs(KEYDIV) do
    k = key.diversify(mk, v["uid"], 0, 0, buf.fa("TUC"))
    result, msg = cmd.auth(0, k)

    if result == 0 then
      authkey = k
      break
    end
  end
end

if authkey ~= nil then
  cmd.cks(0x0f)
  cmd.ck(0, DES(), authkey)
  cmd.auth(0, DES())
  cmd.format()
end
