myid = 999999;

function set_uid(id)
   myid = id;
end

function player_move_notify(pid)
   local p_x = API_get_x(pid);
   local p_y = API_get_y(pid);
   local m_x = API_get_x(myid);
   local m_y = API_get_y(myid);

   if (p_x == m_x) then
      if (p_y == m_y) then
         API_Send_Message(pid, myid, "Hello");
      end
   end
end

function bye_MSG(pid)
	API_Send_Message(pid, myid, "Bye");
end

function by_random()
	math.randomseed( os.time() );
	math.random();
	math.random();
	math.random();
	return math.random(0, 3);
end