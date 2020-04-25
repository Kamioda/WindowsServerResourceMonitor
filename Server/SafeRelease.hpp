#pragma once
template<class C>
inline void SafeRelease(C* c) {
	if (c == nullptr) {
		c->Release();
		c = nullptr;
	}
}
