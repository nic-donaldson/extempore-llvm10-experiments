%mzone = type {i8*, i64, i64, i64, i8*, %mzone*}

define dllexport fastcc i64 @add1_adhoc_W2k2NCxpNjRd__3881(i8* %_impz,i8* %_impenv, i64 %x) nounwind {
entry:
%_impzPtr = alloca i8*
store i8* %_impz, i8** %_impzPtr
%zone3882 = bitcast i8* %_impz to %mzone*
; setup environment
%impenv = bitcast i8* %_impenv to {{i8*, i8*, i64 (i8*, i8*, i64)*}***}*
%add1_adhoc_W2k2NCxpNjRdPtr_ = getelementptr {{i8*, i8*, i64 (i8*, i8*, i64)*}***}, {{i8*, i8*, i64 (i8*, i8*, i64)*}***}* %impenv, i32 0, i32 0
%add1_adhoc_W2k2NCxpNjRdPtr = load {i8*, i8*, i64 (i8*, i8*, i64)*}***, {i8*, i8*, i64 (i8*, i8*, i64)*}**** %add1_adhoc_W2k2NCxpNjRdPtr_

; setup arguments
%xPtr = alloca i64
store i64 %x, i64* %xPtr


%val3883 = load i64, i64* %xPtr
%val3884 = add i64 %val3883, 1
ret i64 %val3884
}
