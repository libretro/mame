-- cv1k subtarget hack
if _OPTIONS["subtarget"]=="arcade" then
	files {
		MAME_DIR .. "src/mame/cave/cv1k_v_blit0.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit1.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit2.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit3.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit4.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit5.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit6.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit7.cpp",
		MAME_DIR .. "src/mame/cave/cv1k_v_blit8.cpp",
	}
end
