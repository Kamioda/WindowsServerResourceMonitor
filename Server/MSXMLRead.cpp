#include "MSXMLRead.hpp"
#include "HandleManager.h"
#include "GetErrorMessage.hpp"
#include <Windows.h>
#include <comdef.h>

namespace MSXML {
	class FileFindHandleManager : public windows::impl::HandleManager<HANDLE> {
	public:
		FileFindHandleManager(HANDLE&& h) : windows::impl::HandleManager<HANDLE>(std::move(h), [](HANDLE& h) { FindClose(h); }) {}
	};
	Read::Read(const std::wstring& FilePath) : lpXmlDoc() {
		{
			WIN32_FIND_DATAW FindData{};
			if (FileFindHandleManager hFind = FindFirstFileW(FilePath.c_str(), &FindData); INVALID_HANDLE_VALUE == hFind) 
				throw std::runtime_error(GetErrorMessageA());
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) throw std::runtime_error("Reserved path is directory");
		}
		VARIANT_BOOL Result{};
		if (const HRESULT hr = this->lpXmlDoc->put_async(VARIANT_FALSE); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
		if (const HRESULT hr = this->lpXmlDoc->load(_variant_t(FilePath.c_str()), &Result); FAILED(hr)) throw std::runtime_error(GetErrorMessageA(hr));
	}

	XmlDataManager::text Read::Load(const std::wstring& Path) {
		if (Base::find(Path) == Base::end()) Base::emplace(this->lpXmlDoc, Path);
		return Base::operator[](Path);
	}
}
