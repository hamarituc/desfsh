function AES(k, v)
  if not k then k = "00000000000000000000000000000000" end
  if not v then v = 0 end
  return { t = "AES", k = k, v = v }
end


function _3K3DES(k, v)
  if not k then k = "000000000000000000000000000000000000000000000000" end
  if not v then v = 0 end
  return { t = "3K3DES", k = k, v = v }
end

function _3DES(k, v)
  if not k then k = "00000000000000000000000000000000" end
  if not v then v = 0 end
  return { t = "3DES", k = k, v = v }
end

function DES(k, v)
  if not k then k = "0000000000000000" end
  if not v then v = 0 end
  return { t = "DES", k = k, v = v }
end
