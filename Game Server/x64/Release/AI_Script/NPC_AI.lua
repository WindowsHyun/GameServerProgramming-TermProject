npc_id = 999999;
npc_level = 0;
npc_hp = 0;
npc_type = -1;


function set_NPC(id, level, hp, type)
   npc_id = id;
   npc_level = level;
   npc_hp = hp;
   npc_type  = type;
end

function player_move_notify(client_id)
   --pid = 클라이언트 아이디
   local p_x = API_get_x(client_id);
   local p_y = API_get_y(client_id);
   local m_x = API_get_x(npc_id);
   local m_y = API_get_y(npc_id);

   if (p_x == m_x) then
      if (p_y == m_y) then
         --API_Send_Message(pid, npc_id, "Hello");
      end
   end
end