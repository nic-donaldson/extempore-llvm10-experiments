%mzone = type {i8*, i64, i64, i64, i8*, %mzone*}
%clsvar = type {i8*, i32, i8*, %clsvar*}

declare i8* @llvm_zone_malloc(%mzone*, i64) nounwind
declare void @llvm_zone_mark(%mzone*) nounwind
declare %clsvar* @new_address_table() nounwind
declare %clsvar* @add_address_table(%mzone*, i8*, i32, i8*, i32, %clsvar*)
declare dllexport fastcc i64 @add1_adhoc_W2k2NCxpNjRd__3881(i8* %_impz,i8* %_impenv, i64 %x) nounwind
declare dllexport fastcc i64 @add1_adhoc_W2k2NCxpNjRd__3908(i8* %_impz,i8* %_impenv, i64 %x) nounwind
declare i64 @llvm_zone_mark_size(%mzone*) nounwind
declare void @llvm_zone_ptr_set_size(i8*, i64) nounwind
declare void @llvm_destroy_zone_after_delay(%mzone*,i64) nounwind
declare %mzone* @llvm_peek_zone_stack() nounwind
declare %mzone* @llvm_pop_zone_stack() nounwind
declare void @llvm_push_zone_stack(%mzone*) nounwind
declare void @llvm_zone_destroy(%mzone*) nounwind

@add1_adhoc_W2k2NCxpNjRd_var = external global [1 x i8*]
@add1_adhoc_W2k2NCxpNjRd_var_zone = external global [1 x i8*]
@gs259 = external global [24 x i8]
@gs260 = external global [35 x i8]

define dllexport ccc {i8*, i8*, i64 (i8*, i8*, i64)*}** @add1_adhoc_W2k2NCxpNjRd_maker(i8* %_impz) nounwind {
entry:
%_impzPtr = alloca i8*
store i8* %_impz, i8** %_impzPtr
%tzone3931 = load i8*, i8** %_impzPtr
%zone3932 = bitcast i8* %tzone3931 to %mzone*

; let assign value to symbol add1_adhoc_W2k2NCxpNjRd
%dat_add1_adhoc_W2k2NCxpNjRd = call i8* @llvm_zone_malloc(%mzone* %zone3932, i64 8)
%add1_adhoc_W2k2NCxpNjRdPtr = bitcast i8* %dat_add1_adhoc_W2k2NCxpNjRd to { i8*, i8*, i64 (i8*, i8*, i64)*}***
%tzone3912 = load i8*, i8** %_impzPtr
%zone3913 = bitcast i8* %tzone3912 to %mzone*
call void @llvm_zone_mark(%mzone* %zone3913)
; malloc closure structure
%clsptr3914 = call i8* @llvm_zone_malloc(%mzone* %zone3913, i64 24)
%closure3915 = bitcast i8* %clsptr3914 to { i8*, i8*, i64 (i8*, i8*, i64)*}*

; malloc environment structure
%envptr3916 = call i8* @llvm_zone_malloc(%mzone* %zone3913, i64 8)
%environment3917 = bitcast i8* %envptr3916 to {{i8*, i8*, i64 (i8*, i8*, i64)*}***}*

; malloc closure address table
%addytable3918 = call %clsvar* @new_address_table()
%var3919 = bitcast [24 x i8]* @gs259 to i8*
%var3920 = bitcast [35 x i8]* @gs260 to i8*
%addytable3921 = call %clsvar* @add_address_table(%mzone* %zone3913, i8* %var3919, i32 0, i8* %var3920, i32 3, %clsvar* %addytable3918)
%address-table3922 = bitcast %clsvar* %addytable3921 to i8*

; insert table, function and environment into closure struct
%closure.table3925 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3915, i32 0, i32 0
store i8* %address-table3922, i8** %closure.table3925
%closure.env3926 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3915, i32 0, i32 1
store i8* %envptr3916, i8** %closure.env3926
%closure.func3927 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3915, i32 0, i32 2
store i64 (i8*, i8*, i64)* @add1_adhoc_W2k2NCxpNjRd__3908, i64 (i8*, i8*, i64)** %closure.func3927
%closure_size3928 = call i64 @llvm_zone_mark_size(%mzone* %zone3913)
call void @llvm_zone_ptr_set_size(i8* %clsptr3914, i64 %closure_size3928)
%wrapper_ptr3929 = call i8* @llvm_zone_malloc(%mzone* %zone3913, i64 8)
%closure_wrapper3930 = bitcast i8* %wrapper_ptr3929 to { i8*, i8*, i64 (i8*, i8*, i64)*}**
store { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3915, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3930

; let value assignment
%add1_adhoc_W2k2NCxpNjRd = select i1 true, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3930, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3930
store { i8*, i8*, i64 (i8*, i8*, i64)*}** %add1_adhoc_W2k2NCxpNjRd, { i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr

; add data to environment
; don't need to alloc for env var add1_adhoc_W2k2NCxpNjRd
%tmp_envptr3924 = getelementptr {{i8*, i8*, i64 (i8*, i8*, i64)*}***}, {{i8*, i8*, i64 (i8*, i8*, i64)*}***}* %environment3917, i32 0, i32 0
store {i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr, {i8*, i8*, i64 (i8*, i8*, i64)*}**** %tmp_envptr3924


%val3933 = load {i8*, i8*, i64 (i8*, i8*, i64)*}**, {i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr
ret {i8*, i8*, i64 (i8*, i8*, i64)*}** %val3933
}


define dllexport ccc void @add1_adhoc_W2k2NCxpNjRd_setter() alwaysinline nounwind {
entry:
%_zone = call ccc %mzone* @llvm_peek_zone_stack()
%_impz = bitcast %mzone* %_zone to i8*
%oldzone1 = getelementptr [1 x i8*], [1 x i8*]* @add1_adhoc_W2k2NCxpNjRd_var_zone, i32 0, i32 0
%oldzone2 = load i8*, i8** %oldzone1
%oldzone3 = bitcast i8* %oldzone2 to %mzone*
store i8* %_impz, i8** %oldzone1
%closure = call ccc {i8*, i8*, i64 (i8*, i8*, i64)*}** @add1_adhoc_W2k2NCxpNjRd_maker(i8* %_impz)
%ptr = bitcast {i8*, i8*, i64 (i8*, i8*, i64)*}** %closure to i8*
%varptr = bitcast [1 x i8*]* @add1_adhoc_W2k2NCxpNjRd_var to i8**
store i8* %ptr, i8** %varptr
; destroy oldzone if not null
%test = icmp ne %mzone* %oldzone3, null
br i1 %test, label %then, label %cont
then:
call ccc void @llvm_destroy_zone_after_delay(%mzone* %oldzone3, i64 441000)
br label %cont
cont:
ret void
}
