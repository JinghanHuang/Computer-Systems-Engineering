
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32
	
	# The size of struct missile
	MISSILE_SIZE = 33

    # Character to draw for an explosion - '@'
	EXPLOSION = 64
	# Character to erase - ' '
	SPACE = 32
	# Character to draw for crosshair - '+'
	CROSSHAIR = 43

	# Crosshair initial position
	SCREEN_CENTER_X = 40
	SCREEN_CENTER_Y = 12
	
	# The bounds of screen
	MIN_X = 0
	MAX_X = 79
	MIN_Y = 0
	MAX_Y = 24
	
	# Constants which helps to calculate offsets
	ROW_OFFSET = 160
	COL_OFFSET = 2
	
	# The positions of cities
	CITY_Y = 24
	CITY_ONE_X = 18
	CITY_TWO_X = 38
	CITY_THREE_X = 58
	
	# to get the high 16 bits, which determines the text-mode video location or to get mp1_score
	SHIFT_OFFSET = 16
	
	# mask to get the status of cities
	MASK_ONE = 0x000000ff
	MASK_TWO = 0x0000ff00
	MASK_THREE = 0x00ff0000
	
	# initial cities' status
	INI_CITIES = 0x00ffffff

# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx 
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
		# save old frame pointer and point to new frame
		pushl %ebp			
		movl %esp, %ebp
		# store callee saved registers
		pushl %ebx
		pushl %esi
		pushl %edi
		
		call update_missiles
		movl %eax, %ebx
		call update_cities
		call update_crosshair
		
		cmpl $0, %ebx
		je mp1_rtc_tasklet_done
		call mp1_notify_user
		
	mp1_rtc_tasklet_done:
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		# load cmd value
		movl 8(%esp), %ecx
		
		# check if cmd value between 0 and 4
		cmpl $0, %ecx
		jl mp1_ioctl_invalid_cmd
		cmpl $4, %ecx
		jg mp1_ioctl_invalid_cmd
		
		# call core function, we use 4 bytes stores a command  
		jmp *jumptable(,%ecx,4)
		jmp mp1_ioctl_done
		
	mp1_ioctl_invalid_cmd:
		# return -1
		movl $-1, %eax

	mp1_ioctl_done:
		ret


# ----------------- Functions private to this file -------------------


# void update_missiles()
# the first helper function of mp1_rtc_tasklet
# you can walk down the struct missile linked list,
# and update the status of all the missile
#
# Inputs: none
# Outputs: edi stores a value, which indicates whether we need to call mp1_notify_user 
# Registers: Clobbers ECX
update_missiles:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
		movl mp1_missile_list, %ebx
		# let edi equal to zero, which stores the return value of missile explode
		movl $0, %edi
		
	update_missiles_loop:
		# check if the missile is empty
		cmpl $0, %ebx
		je update_missiles_done
		# check whether the missile is currently exploding
		cmpl $0, EXPLODED(%ebx)
		jne update_missiles_exploding
		
		# clean old position
		movb $SPACE, %cl
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		imull $ROW_OFFSET, %esi
		movl X(%ebx), %eax
		shrl $SHIFT_OFFSET, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# update missile position
		movl VX(%ebx), %esi
		addl %esi, X(%ebx)
		movl VY(%ebx), %esi
		addl %esi, Y(%ebx)
		
		# check if the missile has moved off of the screen
	update_missiles_check_left:
		movl X(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		cmpl $MIN_X, %esi
		jge update_missiles_check_right
		call mp1_missile_remove
		jmp update_missiles_loop
	update_missiles_check_right:
		cmpl $MAX_X, %esi
		jle update_missiles_check_down
		call mp1_missile_remove
		jmp update_missiles_loop
		
	update_missiles_check_down:
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		cmpl $MIN_Y, %esi
		jge update_missiles_check_up
		call mp1_missile_remove
		jmp update_missiles_loop
	update_missiles_check_up:
		cmpl $MAX_Y, %esi
		jle update_missiles_check_dest
		call mp1_missile_remove
		jmp update_missiles_loop
		
	update_missiles_check_dest:
		# check if the missile has reached its destination 
		movl X(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		cmpl DEST_X(%ebx), %esi
		jne update_missiles_moving
		
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		cmpl DEST_Y(%ebx), %esi
		jne update_missiles_moving

		
	update_missiles_exploding:
	# missile needs to be exploding or is currently exploding
		push %ebx
		call missile_explode
		addl $4, %esp
		cmpl $0, %eax
		je update_missiles_remove_check
		movl %eax, %edi
		
	update_missiles_remove_check:
	# check if exploded field reaches zero
		subl $1, EXPLODED(%ebx)
		cmpl $0, EXPLODED(%ebx)
		jne update_missiles_draw_explosion
		# erase explosion
		movb $SPACE, %cl
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		imull $ROW_OFFSET, %esi
		movl X(%ebx), %eax
		shrl $SHIFT_OFFSET, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# remove from list
		call mp1_missile_remove
		jmp update_missiles_loop
		
	update_missiles_draw_explosion:
	# draw explosion character
		movb $EXPLOSION, %cl
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		imull $ROW_OFFSET, %esi
		movl X(%ebx), %eax
		shrl $SHIFT_OFFSET, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		jmp update_missiles_next
		
	update_missiles_moving:
	# the missile is still moving - draw new position
		movb C(%ebx), %cl
		movl Y(%ebx), %esi
		shrl $SHIFT_OFFSET, %esi
		imull $ROW_OFFSET, %esi
		movl X(%ebx), %eax
		shrl $SHIFT_OFFSET, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
	update_missiles_next:
	# check the next missile
		movl NEXT(%ebx), %ebx
		jmp update_missiles_loop
		
		
	update_missiles_done:
		movl %edi, %eax
	
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# void mp1_missile_remove(struct missile* user missile);)
# a helper function
# you can remove the missile from the linked list,
# and free it with mp1 free
# 
# Inputs: none
# Outputs: edi stores a value, which indicates whether we need to call mp1_notify_user 
# Registers: Clobbers ECX
mp1_missile_remove:
		pushl %ebp			
		movl %esp, %ebp
		pushl %esi
		pushl %edi
		
		# check if the node is head	
		movl mp1_missile_list, %esi
		cmpl %esi, %ebx
		jne mp1_missile_remove_findnext

		# remove head
		movl NEXT(%ebx), %esi
		movl %esi, mp1_missile_list
		pushl %ebx
		call mp1_free
		addl $4, %esp
		movl mp1_missile_list, %ebx
		jmp mp1_missile_remove_done
		
	mp1_missile_remove_findnext:
		# find the node in the linked list
		movl NEXT(%esi), %edi
		cmpl %edi, %ebx
		je mp1_missile_remove_noderemove
		movl %edi, %esi
		jmp mp1_missile_remove_findnext
		
	mp1_missile_remove_noderemove:
		movl NEXT(%edi), %ecx
		movl %ecx, NEXT(%esi)
		pushl %ebx
		call mp1_free
		addl $4, %esp
		movl NEXT(%esi), %ebx
		
	mp1_missile_remove_done:
		popl %edi
		popl %esi
		leave
		ret
		
		
# void update_cities()
# the second helper function of mp1_rtc_tasklet
# update the status of all the missile
# redraw cities
#
# Inputs: none
# Outputs: none
# Registers: Clobbers ECX
update_cities:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
	# for city 1
		# esi - counter
		movl $0, %esi	
	draw_city_one_check:
		movl base_alive, %ebx
		# get the status of city one
		andl $MASK_ONE, %ebx 
		cmpl $0, %ebx
		je draw_destroyed_city_one
		# get alive base picture
		movb base_pic(%esi), %cl
		jmp draw_city_one
		
	draw_destroyed_city_one:
		# get destroyed base picture
		movb dead_base_pic(%esi), %cl
	
	draw_city_one:
		# draw the city
		movl $CITY_Y, %edi
		imull $ROW_OFFSET, %edi
		movl $CITY_ONE_X, %eax
		addl %esi, %eax
		imull $COL_OFFSET, %eax
		addl %edi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# check if drawing 5 characters
		addl $1, %esi
		cmpl $5, %esi
		jl draw_city_one_check
		
		
	# for city 2
		# esi - counter
		movl $0, %esi
	draw_city_two_check:
		movl base_alive, %ebx
		# get the status of city two
		andl $MASK_TWO, %ebx
		cmpl $0, %ebx
		je draw_destroyed_city_two
		# get alive base picture
		movb base_pic(%esi), %cl
		jmp draw_city_two
		
	draw_destroyed_city_two:
		# get destroyed base picture
		movb dead_base_pic(%esi), %cl
	
	draw_city_two:
		# draw the city
		movl $CITY_Y, %edi
		imull $ROW_OFFSET, %edi
		movl $CITY_TWO_X, %eax
		addl %esi, %eax
		imull $COL_OFFSET, %eax
		addl %edi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# check if drawing five characters
		addl $1, %esi
		cmpl $5, %esi
		jl draw_city_two_check
		
	# for city 3
		# esi - counter
		movl $0, %esi
	draw_city_three_check:
		movl base_alive, %ebx
		# get the status of city three
		andl $MASK_THREE, %ebx
		cmpl $0, %ebx
		je draw_destroyed_city_three
		# get alive base picture
		movb base_pic(%esi), %cl
		jmp draw_city_three
		
	draw_destroyed_city_three:
		# get destroyed base picture
		movb dead_base_pic(%esi), %cl
	
	draw_city_three:
		# draw the city
		movl $CITY_Y, %edi
		imull $ROW_OFFSET, %edi
		movl $CITY_THREE_X, %eax
		addl %esi, %eax
		imull $COL_OFFSET, %eax
		addl %edi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# check if drawing five characters
		addl $1, %esi
		cmpl $5, %esi
		jl draw_city_three_check
		
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

		
# void update_crosshair()
# the third helper function of mp1_rtc_tasklet
# update the status of all the cross hair
# redraw cross hair
#
# Inputs: none
# Outputs: none
# Registers: Clobbers ECX
update_crosshair:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi

		# draw the cross hair
		movb $CROSSHAIR, %cl
		movl crosshairs_y, %edi
		imull $ROW_OFFSET, %edi
		movl crosshairs_x, %eax
		imull $COL_OFFSET, %eax
		addl %edi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# int mp1 ioctl startgame (unsigned long ignore)
# This function is called when the game is about to start.
# the crosshairs should be set to the middle of the screen: (40, 12).
#
# Inputs: unsigned long ignore - meaningless
# Outputs: none
mp1_ioctl_startgame:
		movl $0x0, mp1_missile_list # initialize list pointer
		movl $INI_CITIES, base_alive	# initialize alive bases
		movl $0x0, mp1_score	# initialize player's current score

		# initialize cross hair
		movl $SCREEN_CENTER_X, crosshairs_x
		movl $SCREEN_CENTER_Y, crosshairs_y
		ret

		
# int mp1 ioctl addmissile (struct missile* user missile)
# This ioctl must add a new missile to the game
# This function needs to copy the user’s missile into 
# a dynamically allocated buffer in kernel space.
#
# Inputs: struct missile* user missile - a pointer to a struct missile in user space
# Outputs: 0 - succeed
#		   -1 - fail
# Registers: Standard C calling convention
mp1_ioctl_addmissile:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
		# load user missile
		movl 8(%ebp), %ebx
		
		# dynamic memory allocation
		pushl $MISSILE_SIZE
		call mp1_malloc
		addl $4, %esp
		
		movl %eax, %esi
		cmpl $0, %esi
		je mp1_ioctl_addmissile_fail
		
		# data copy
		pushl $MISSILE_SIZE
		pushl %ebx
		pushl %eax
		call mp1_copy_from_user
		addl $12, %esp
		cmpl $0, %eax
		jne mp1_ioctl_addmissile_fail
		
		# changing the head of the list
		movl mp1_missile_list,%edi
		movl %edi, NEXT(%esi)
		movl %esi, mp1_missile_list
		
		# return 0
		movl $0, %eax
		jmp mp1_ioctl_addmissile_done
		
	mp1_ioctl_addmissile_fail:		
		pushl %esi
		call mp1_free
		addl $4, %esp
		# return -1
		movl $-1, %eax
	
	mp1_ioctl_addmissile_done:
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# int mp1 ioctl movexhairs (unsigned long xhair delta packed)
# This function moves the crosshairs
# If the position of the crosshairs does change,
# this function should redraw it at its new location.
#
# Inputs: unsigned long xhair delta packed - a 32-bit integer containing 
# two signed 16-bit integers packed into its low and high words
# Outputs: 0
# Registers: Clobbers ECX
mp1_ioctl_movexhairs:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
		# initialize ebx and edi
		movl $0, %ebx
		movl $0, %edi
		
		# load xhair delta packed
		movw 8(%ebp), %bx
		movw 10(%ebp), %di
		# sign extend
		movsx %bx, %ebx
		movsx %di, %edi
		
		# clean old movexhair
		movb $SPACE, %cl
		movl crosshairs_y, %esi
		imull $ROW_OFFSET, %esi
		movl crosshairs_x, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# move cross hair
		addl %ebx, crosshairs_x
		addl %edi, crosshairs_y
		
		# check bounds
	mp1_ioctl_movexhairs_check_left:
		cmpl $MIN_X, crosshairs_x
		jge mp1_ioctl_movexhairs_check_right
		movl $MIN_X, crosshairs_x
		
	mp1_ioctl_movexhairs_check_right:
		cmpl $MAX_X, crosshairs_x
		jle mp1_ioctl_movexhairs_check_down
		movl $MAX_X, crosshairs_x
		
	mp1_ioctl_movexhairs_check_down:
		cmpl $MIN_Y, crosshairs_y
		jge mp1_ioctl_movexhairs_check_up
		movl $MIN_Y, crosshairs_y
	
	mp1_ioctl_movexhairs_check_up:
		cmpl $MAX_Y, crosshairs_y
		jle mp1_ioctl_movexhairs_draw_new
		movl $MAX_Y, crosshairs_y
	
	mp1_ioctl_movexhairs_draw_new:
		# draw new cross hair
		movb $CROSSHAIR, %cl
		movl crosshairs_y, %esi
		imull $ROW_OFFSET, %esi
		movl crosshairs_x, %eax
		imull $COL_OFFSET, %eax
		addl %esi, %eax
		
		pushl %edx
		call mp1_poke
		popl %edx
		
		# return 0
		movl $0, %eax
		
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# int mp1 ioctl getstatus (unsigned long* user status)
# This function should copy the current score into the low 16-bits of that integer, and the
# status of the three cities into bits 16, 17, and 18.
#
# Inputs: unsigned long* user status - a pointer to a 32-bit integer in user space
# two signed 16-bit integers packed into its low and high words
# Outputs: 0 - succeed
#          -1 - fail
# Registers: Standard C calling convention
mp1_ioctl_getstatus:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
		# load user status
		movl 8(%ebp), %ebx
		
		# initialize edi, which will store information
		# we need to copy to user space
		movl $0, %edi
		
	mp1_ioctl_getstatus_city_three:
		movl base_alive, %esi
		# get the status of city three
		andl $MASK_THREE, %esi
		cmpl $0, %esi
		je mp1_ioctl_getstatus_city_two
		addl $1, %edi
		
	mp1_ioctl_getstatus_city_two:
		shll $1, %edi
		movl base_alive, %esi
		# get the status of city two
		andl $MASK_TWO, %esi
		cmpl $0, %esi
		je mp1_ioctl_getstatus_city_one
		addl $1, %edi
		
	mp1_ioctl_getstatus_city_one:
		shll $1, %edi
		movl base_alive, %esi
		# get the status of city one
		andl $MASK_ONE, %esi
		cmpl $0, %esi
		je mp1_ioctl_getstatus_score
		addl $1, %edi
		
	mp1_ioctl_getstatus_score:
		# left shift 16 bit to store mp1_score
		shll $SHIFT_OFFSET, %edi
		movl mp1_score, %ecx
		addl %ecx, %edi
		
		# get the address of edi
		pushl %edi 
		movl %esp, %edi
		
		pushl $4 # the size of integer is 4 bytes
		pushl %edi
		pushl %ebx	
		call mp1_copy_to_user
		addl $12, %esp
		popl %edi
		cmpl $0, %eax
		je mp1_ioctl_getstatus_success
			
		# return -1
		movl $-1, %eax
		jmp mp1_ioctl_getstatus_done
		
	mp1_ioctl_getstatus_success:
		# return 0
		movl $0, %eax
		
	mp1_ioctl_getstatus_done:
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret
		

# int mp1 ioctl endgame (unsigned long ignore)
# Called when the game is over, this function must perform all the cleanup work. 
# It should free all the memory being used by the linked list and then return success.
#
# Inputs: unsigned long ignore - meaningless
# Outputs: none
# Registers: Standard C calling convention
mp1_ioctl_endgame:
		pushl %ebp			
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		
	mp1_ioctl_endgame_clean_loop:
		cmpl $0, mp1_missile_list
		je mp1_ioctl_endgame_done
		movl mp1_missile_list, %ebx
		movl NEXT(%ebx), %esi
		movl %esi, mp1_missile_list
		pushl %ebx
		call mp1_free
		addl $4, %esp
		jmp mp1_ioctl_endgame_clean_loop
		
	mp1_ioctl_endgame_done:
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

jumptable:
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame
	

	
