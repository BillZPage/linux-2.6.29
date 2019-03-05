##main function defined for initial the S3C2440 CPU.

define reset_2440 
	monitor endian little  ##little endian, should be the same as your application.
	
	##copied from JLinkGDBServer Document.
	monitor reset            ##reset the S3C2440
	#monitor reg cpsr = 0xd3  ##setup cpsr register.
	monitor speed auto       ##Link Speed.
	
	##translated from VIVI S3C2440 version.
	#disable watchdog
	monitor MemU32 0x53000000 = 0
	
	#disalbe interrupt --- int-mask register 
	monitor MemU32 0x4A000008 = 0xFFFFFFFF
	
	#disalbe interrupt --- int-sub-mask register 
	monitor MemU32 0x4A00001C = 0x000007FF
	
	#initialize system clocks --- locktime register
	monitor long 0x4C000000 = 0x00FFFFFF
		
	#initialize system clocks --- mpll register
	monitor long 0x4C000004 = 0x0005C042
	
	#setup memory controller
	monitor MemU32 0x48000000 = 0x22111120
	monitor MemU32 0x48000004 = 0x00000700
	monitor MemU32 0x48000008 = 0x00000700
	monitor MemU32 0x4800000c = 0x00000700
	monitor MemU32 0x48000000 = 0x22111120
	monitor MemU32 0x48000004 = 0x00000700
	monitor MemU32 0x48000008 = 0x00000700
	monitor MemU32 0x4800000c = 0x00000700
	monitor MemU32 0x48000010 = 0x00000700
	monitor MemU32 0x48000014 = 0x00000700
	monitor MemU32 0x48000018 = 0x00000700
	monitor MemU32 0x4800001c = 0x00018005
	monitor MemU32 0x48000020 = 0x00018005
	monitor MemU32 0x48000024 = 0x008E0459
	monitor MemU32 0x48000028 = 0x00000032
	monitor MemU32 0x4800002c = 0x30      
	monitor MemU32 0x48000030 = 0x30     

	#setup mmu
	#monitor MemU32 0x33f03000 = 0x30000c12 
	#monitor MemU32 0x33f03004 = 0x30100c12 
	#monitor MemU32 0x33f03008 = 0x30200c12 
	#monitor MemU32 0x33f0300c = 0x30300c12 
	#monitor MemU32 0x33f03010 = 0x30400c12 
	#monitor MemU32 0x33f03014 = 0x30500c12 
	#monitor MemU32 0x33f03018 = 0x30600c12 
	#monitor MemU32 0x33f0301c = 0x30700c12 
	#monitor MemU32 0x33f00cfc = 0x33f00c12 
	#monitor cp15 2 0 0 0 = 0x33f00000
	#monitor cp15 1 0 0 0 = 0x7171 
	
end

##main function defined for connect to the TARGET.
##arg0 = IP address.
##arg1 = PORT number of JLINK gdb server.

define connect_jei 
  if $argc == 2
      target remote $arg0:$arg1
      reset_2440
  end
end