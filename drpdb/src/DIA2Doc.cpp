//the DIA documentation is unreliable and out of date
//this file automatically determines which functions are valid to call on which symbol types.
//this file must be updated when the DIA api is modified
#include "DiaCommon.h"
#include <functional>
#include <iostream>
#include <windows.h>
#include "drpdb.h"
#include <algorithm>
#include <array>
#include <fstream>
namespace DIA2
{

	template<class PSymbols>
	void MeasureSymbols(IDiaSession* session, PSymbols f_syms)
	{
		CComPtr<IDiaEnumTables> streamEnum = nullptr;
		session->getEnumTables(&streamEnum);
		while (streamEnum)
		{
			ULONG numFetched = 1;
			CComPtr<IDiaTable> table = nullptr;
			streamEnum->Next(numFetched, &table, &numFetched);
			if (table == nullptr || numFetched < 1)
				break;
			CComBSTR enumDataName;
			table->get_name(&enumDataName);
			if (wcscmp(enumDataName, DiaTable_Symbols) == 0)
			{
				CComPtr<IDiaEnumSymbols> pEnumSym = nullptr;
				if (S_OK == table->QueryInterface(__uuidof(IDiaEnumSymbols), (void**)&pEnumSym))
				{
					DIA2::ReadTable<IDiaSymbol>(pEnumSym, f_syms, "symbols");
				}
			}
		}
	}

	template<class T> constexpr typename std::enable_if_t<!std::is_pointer<T>::value, T> map_type(HRESULT(IDiaSymbol::*)(T*));
	constexpr CComVariant map_type(HRESULT(IDiaSymbol::*)(VARIANT*));
	constexpr CComBSTR map_type(HRESULT(IDiaSymbol::*)(BSTR*));
	template<class T> constexpr typename std::enable_if_t<std::is_pointer<T>::value, CComPtr<std::remove_pointer_t<T>>> map_type(HRESULT(IDiaSymbol::*)(T*));
	struct dword_buffer { using type = DWORD; };
	struct byte_buffer { using type = BYTE; };
	struct word_buffer { using type = WORD;  };
	constexpr dword_buffer map_type(HRESULT(IDiaSymbol::*)(DWORD, DWORD*, DWORD*));
	constexpr byte_buffer  map_type(HRESULT(IDiaSymbol::*)(DWORD, DWORD*, BYTE*));
	constexpr word_buffer  map_type(HRESULT(IDiaSymbol::*)(DWORD, DWORD*, WORD*));

	template<class T>
	struct is_buf{	enum { value = false};};
	template<>	struct is_buf<dword_buffer> { enum { value = true };  };
	template<>	struct is_buf<byte_buffer>{	enum {value = true}; };
	template<>  struct is_buf<word_buffer>{ enum {value = true}; };
	struct APITest
	{
		typedef std::vector<const char*> stringlist;
		DIA2::Provider provider;
		std::array<stringlist, SymTagMax>& measured;
		APITest(std::array<stringlist, SymTagMax>& log)
			:measured(log)
		{}
		void add_measured(const char* str, std::vector<const char*>& found)
		{
			found.push_back(str);
		}
		
		template<class T, class U>
		std::enable_if_t<!is_buf<T>::value, void> default_measure(stringlist& strings, const char* str, U call)
		{
			if (std::find(strings.begin(), strings.end(), str) == strings.end())
			{
				T D;
				if (S_OK == call(&D))
				{
					add_measured(str, strings);
				}
			}
		}
		template<class T, class U>
		std::enable_if_t<is_buf<T>::value, void> default_measure(stringlist& strings, const char* str, U call)
		{
			if (std::find(strings.begin(), strings.end(), str) == strings.end())
			{
				T::type buf[1];
				DWORD n;
				if (S_OK == call(1,&n,buf))
				{
					add_measured(str, strings);
				} 
			}
		}
		void init(LONG) {}
#define MEASURE(func) default_measure<decltype( map_type(&IDiaSymbol::##func))>(found, #func, [symbol](auto&&... p){return symbol->##func(p...);} );

		void element(IDiaSymbol* symbol)
		{
			
			DWORD type = 0;
			symbol->get_symTag(&type);
			auto& found = measured[type];

			MEASURE(get_symIndexId)
			MEASURE(get_name)
			MEASURE(get_lexicalParent)
			MEASURE(get_classParent)
			MEASURE(get_type)
			MEASURE(get_dataKind)
			MEASURE(get_locationType)
			MEASURE(get_addressSection)
			MEASURE(get_addressOffset)
			MEASURE(get_relativeVirtualAddress)
			MEASURE(get_virtualAddress)
			MEASURE(get_registerId)
			MEASURE(get_offset)
			MEASURE(get_length)
			MEASURE(get_slot)
			MEASURE(get_volatileType)
			MEASURE(get_constType)
			MEASURE(get_unalignedType)
			MEASURE(get_access)
			MEASURE(get_libraryName)
			MEASURE(get_platform)
			MEASURE(get_language)
			MEASURE(get_editAndContinueEnabled)
			MEASURE(get_frontEndMajor)
			MEASURE(get_frontEndMinor)
			MEASURE(get_frontEndBuild)
			MEASURE(get_backEndMajor)
			MEASURE(get_backEndMinor)
			MEASURE(get_backEndBuild)
			MEASURE(get_sourceFileName)
			MEASURE(get_unused)
			MEASURE(get_thunkOrdinal)
			MEASURE(get_thisAdjust)
			MEASURE(get_virtualBaseOffset)
			MEASURE(get_virtual)
			MEASURE(get_intro)
			MEASURE(get_pure)
			MEASURE(get_callingConvention)
			MEASURE(get_value)
			MEASURE(get_baseType)
			MEASURE(get_token)
			MEASURE(get_timeStamp)
			MEASURE(get_guid)
			MEASURE(get_symbolsFileName)
			MEASURE(get_reference)
			MEASURE(get_count)
			MEASURE(get_bitPosition)
			MEASURE(get_arrayIndexType)
			MEASURE(get_packed)
			MEASURE(get_constructor)
			MEASURE(get_overloadedOperator)
			MEASURE(get_nested)
			MEASURE(get_hasNestedTypes)
			MEASURE(get_hasAssignmentOperator)
			MEASURE(get_hasCastOperator)
			MEASURE(get_scoped)
			MEASURE(get_virtualBaseClass)
			MEASURE(get_indirectVirtualBaseClass)
			MEASURE(get_virtualBasePointerOffset)
			MEASURE(get_virtualTableShape)
			MEASURE(get_lexicalParentId)
			MEASURE(get_classParentId)
			MEASURE(get_typeId)
			MEASURE(get_arrayIndexTypeId)
			MEASURE(get_virtualTableShapeId)
			MEASURE(get_code)
			MEASURE(get_function)
			MEASURE(get_managed)
			MEASURE(get_msil)
			MEASURE(get_virtualBaseDispIndex)
				
			//MEASURE(get_undecoratedNameEx) duplicate of get_undecoratedName
			MEASURE(get_undecoratedName)

			MEASURE(get_age)
			MEASURE(get_signature)
			MEASURE(get_compilerGenerated)
			MEASURE(get_addressTaken)
			MEASURE(get_rank)
			MEASURE(get_lowerBound)
			MEASURE(get_upperBound)
			MEASURE(get_lowerBoundId)
			MEASURE(get_upperBoundId)
			MEASURE(get_dataBytes)
			MEASURE(get_targetSection)
			MEASURE(get_targetOffset)
			MEASURE(get_targetRelativeVirtualAddress)
			MEASURE(get_targetVirtualAddress)
			MEASURE(get_machineType)
			MEASURE(get_oemId)
			MEASURE(get_oemSymbolId)

			//MEASURE(get_types)	duplicate of get_typeIds
			MEASURE(get_typeIds)

			MEASURE(get_objectPointerType)
			MEASURE(get_udtKind)
			MEASURE(get_noReturn)
			MEASURE(get_customCallingConvention)
			MEASURE(get_noInline)
			MEASURE(get_optimizedCodeDebugInfo)
			MEASURE(get_notReached)
			MEASURE(get_interruptReturn)
			MEASURE(get_farReturn)
			MEASURE(get_isStatic)
			MEASURE(get_hasDebugInfo)
			MEASURE(get_isLTCG)
			MEASURE(get_isDataAligned)
			MEASURE(get_hasSecurityChecks)
			MEASURE(get_compilerName)
			MEASURE(get_hasAlloca)
			MEASURE(get_hasSetJump)
			MEASURE(get_hasLongJump)
			MEASURE(get_hasInlAsm)
			MEASURE(get_hasEH)
			MEASURE(get_hasSEH)
			MEASURE(get_hasEHa)
			MEASURE(get_isNaked)
			MEASURE(get_isAggregated)
			MEASURE(get_isSplitted)
			MEASURE(get_container)
			MEASURE(get_inlSpec)
			MEASURE(get_noStackOrdering)
			MEASURE(get_virtualBaseTableType)
			MEASURE(get_hasManagedCode)
			MEASURE(get_isHotpatchable)
			MEASURE(get_isCVTCIL)
			MEASURE(get_isMSILNetmodule)
			MEASURE(get_isCTypes)
			MEASURE(get_isStripped)
			MEASURE(get_frontEndQFE)
			MEASURE(get_backEndQFE)
			MEASURE(get_wasInlined)
			MEASURE(get_strictGSCheck)
			MEASURE(get_isCxxReturnUdt)
			MEASURE(get_isConstructorVirtualBase)
			MEASURE(get_RValueReference)
			MEASURE(get_unmodifiedType)
			MEASURE(get_framePointerPresent)
			MEASURE(get_isSafeBuffers)
			MEASURE(get_intrinsic)
			MEASURE(get_sealed)
			MEASURE(get_hfaFloat)
			MEASURE(get_hfaDouble)
			MEASURE(get_liveRangeStartAddressSection)
			MEASURE(get_liveRangeStartAddressOffset)
			MEASURE(get_liveRangeStartRelativeVirtualAddress)
			MEASURE(get_countLiveRanges)
			MEASURE(get_liveRangeLength)
			MEASURE(get_offsetInUdt)
			MEASURE(get_paramBasePointerRegisterId)
			MEASURE(get_localBasePointerRegisterId)
			MEASURE(get_isLocationControlFlowDependent)
			MEASURE(get_stride)
			MEASURE(get_numberOfRows)
			MEASURE(get_numberOfColumns)
			MEASURE(get_isMatrixRowMajor)
			MEASURE(get_numericProperties)
			MEASURE(get_modifierValues)
			MEASURE(get_isReturnValue)
			MEASURE(get_isOptimizedAway)
			MEASURE(get_builtInKind)
			MEASURE(get_registerType)
			MEASURE(get_baseDataSlot)
			MEASURE(get_baseDataOffset)
			MEASURE(get_textureSlot)
			MEASURE(get_samplerSlot)
			MEASURE(get_uavSlot)
			MEASURE(get_sizeInUdt)
			MEASURE(get_memorySpaceKind)
			MEASURE(get_unmodifiedTypeId)
			MEASURE(get_subTypeId)
			MEASURE(get_subType)
			MEASURE(get_numberOfModifiers)
			MEASURE(get_numberOfRegisterIndices)
			MEASURE(get_isHLSLData)
			MEASURE(get_isPointerToDataMember)
			MEASURE(get_isPointerToMemberFunction)
			MEASURE(get_isSingleInheritance)
			MEASURE(get_isMultipleInheritance)
			MEASURE(get_isVirtualInheritance)
			MEASURE(get_restrictedType)
			MEASURE(get_isPointerBasedOnSymbolValue)
			MEASURE(get_baseSymbol)
			MEASURE(get_baseSymbolId)
			MEASURE(get_objectFileName)
			MEASURE(get_isSdl)
			MEASURE(get_isWinRTPointer)
			MEASURE(get_isRefUdt)
			MEASURE(get_isValueUdt)
			MEASURE(get_isInterfaceUdt)
			MEASURE(findInlineeLines)
			MEASURE(getSrcLineOnTypeDefn)
			MEASURE(get_isPGO)
			MEASURE(get_hasValidPGOCounts)
			MEASURE(get_isOptimizedForSpeed)
			MEASURE(get_PGOEntryCount)
			MEASURE(get_PGOEdgeCount)
			MEASURE(get_PGODynamicInstructionCount)
			MEASURE(get_staticSize)
			MEASURE(get_finalLiveStaticSize)
			MEASURE(get_phaseName)
			MEASURE(get_hasControlFlowCheck)
			MEASURE(get_constantExport)
			MEASURE(get_dataExport)
			MEASURE(get_privateExport)
			MEASURE(get_noNameExport)
			MEASURE(get_exportHasExplicitlyAssignedOrdinal)
			MEASURE(get_exportIsForwarder)
			MEASURE(get_ordinal)
			MEASURE(get_frameSize)
			MEASURE(get_exceptionHandlerAddressSection)
			MEASURE(get_exceptionHandlerAddressOffset)
			MEASURE(get_exceptionHandlerRelativeVirtualAddress)
			MEASURE(get_exceptionHandlerVirtualAddress)
			MEASURE(findInputAssemblyFile)
			MEASURE(get_characteristics)
			MEASURE(get_coffGroup)
		};
	};

	void Document()
	{
		std::ifstream infile("../config/doc/in.txt");
		auto intext = std::string(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
		std::vector<std::string> files;
		auto pos = intext.begin();
		while (pos != intext.end())
		{
			auto next = std::find(pos, intext.end(), '\n');
			files.emplace_back(pos, next);
			if (next != intext.end())
				++next;
			pos = next;
		}
		
		HMODULE hModule = GetModuleHandleA(NULL);
		CHAR path[MAX_PATH];
		GetModuleFileNameA(hModule, path, MAX_PATH);
		files.push_back(path);
		std::array<std::vector<const char*>, SymTagMax> measured;
		for (auto file : files)
		{
			APITest tester(measured);
			tester.provider.load(file.c_str());
			DIA2::MeasureSymbols(tester.provider.session, tester);
		}

		std::ofstream outfile("../config/doc/out.txt");

		int i = 0;
		for (auto& elem : measured)
		{
			std::sort(elem.begin(), elem.end(), [](auto a, auto b) {return strcmp(a, b) < 0; });
			outfile << i << ": ";
			for (auto& got : elem)
			{
				outfile << got << " ";
			}
			outfile << std::endl;
			++i;
		}

	}
}