-- cv1k/ep1c12 subtarget hack
if _OPTIONS["subtarget"]=="arcade" then
	files {
		MAME_DIR .. "src/mame/cave/ep1c12.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12.h",
		MAME_DIR .. "src/mame/cave/ep1c12_blit0.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit1.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit2.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit3.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit4.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit5.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit6.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit7.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12_blit8.cpp",
		MAME_DIR .. "src/mame/cave/ep1c12in.ipp",
		MAME_DIR .. "src/mame/cave/ep1c12pixel.ipp",
	}
end
