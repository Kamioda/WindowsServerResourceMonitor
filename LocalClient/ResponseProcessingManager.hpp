#pragma once
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GaugeValueManager.hpp"
#include "StringController.hpp"
#include "StringManager.hpp"
#include "Color.hpp"
#include <picojson.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <functional>

class ResponseProcessingManager {
private:
	static constexpr size_t CharBufferSize = 1024;
	class Base {
	public:
		class GraphicInformation {
		private:
			GraphicHandle handle;
		public:
			int Radius;
		private:
			Color Background;
			int GaugeWidth;
			double DrawStartPos;
			double NoUseArea;
			static constexpr int MaxVertexOfTriangle = 120;
			static constexpr int digit(const double val) { return static_cast<int>(val + 0.5); }
			static constexpr double ToRadian(const double Vertex) { return Vertex * M_PI / 180; }
			static inline double GetSinVal(const double Vertex) { return std::sin(ToRadian(Vertex)); }
			static inline double GetCosVal(const double Vertex) { return std::cos(ToRadian(Vertex)); }
			//inline void DrawTriangleImpl(const int X, const int Y, const double CenterVertex, const int Perpendicular, const double RightEndPointVertex, const bool Trans) const noexcept {
			//	const double LineLength = static_cast<double>(Perpendicular) / GetSinVal(90.0 - (CenterVertex / 2));
			//	const int RightEndPointX = X + digit(LineLength * GetCosVal(RightEndPointVertex));
			//	const int RightEndPointY = Y + digit(LineLength * GetSinVal(RightEndPointVertex));
			//	const int LeftEndPointX = X + digit(LineLength * GetCosVal(RightEndPointVertex + CenterVertex));
			//	const int LeftEndPointY = Y + digit(LineLength * GetSinVal(RightEndPointVertex + CenterVertex));
			//	DxLib::DrawTriangle(X, Y, LeftEndPointX, LeftEndPointY, RightEndPointX, RightEndPointY, /*this->Background.GetColorCode()*/0, Trans ? FALSE : TRUE);
			//}
			//inline void DrawTriangle(const int X, const int Y, const int CenterVertex, const int Perpendicular, const int RightEndPointVertex, const bool Trans) const noexcept {
			//	for (int i = CenterVertex, j = RightEndPointVertex; i > 0; i -= MaxVertexOfTriangle, j -= MaxVertexOfTriangle)
			//		DrawTriangleImpl(X, Y, static_cast<double>(std::min(i, MaxVertexOfTriangle)), Perpendicular, static_cast<double>(-j), Trans);
			//}
			double CalcDrawPercent(const double Percent) {
				return this->DrawStartPos - (Percent * (100.0 - this->NoUseArea) / 100.0);
			}
		public:
			GraphicInformation(const std::string& FilePath, const std::string& BackgroundColor = "#ffffff", const int GaugeWidth = 10, const double DrawStartPos = -25.0, const double NoUseArea = 50.0)
				: handle(DxLib::LoadGraph(CharsetManager::AlignCmdLineStrType(FilePath).c_str())), Background(BackgroundColor), GaugeWidth(GaugeWidth), DrawStartPos(DrawStartPos), NoUseArea(NoUseArea) {
				if (this->handle == -1) throw std::runtime_error("Failed to load graph image\nPath : " + FilePath);
				int X = 0, Y = 0;
				DxLib::GetGraphSize(this->handle, &X, &Y);
				this->Radius = X / 2;
			}
			void Draw(const int X, const int Y, const double Percent) const noexcept {
				static const Color Black = Color("#000000");
				// Draw関数の引数が左上座標なので半径を足すことで場所を調整

				// 黒いエリアのサイズを調整するために-3を加えている
				DrawCircle(X + this->Radius, Y + this->Radius, this->Radius - 3, Black.GetColorCode());
				DrawCircleGauge(X + this->Radius, Y + this->Radius, Percent, this->handle, this->DrawStartPos);
				DrawCircle(X + this->Radius, Y + this->Radius, this->Radius - this->GaugeWidth, this->Background.GetColorCode());
			}
		};
		class ResponsePercentDataProcessor {
		protected:
			GaugeValueManager<int> Val;
			GraphicInformation GraphInfo;
			std::reference_wrapper<StringManager> string;
		public:
			ResponsePercentDataProcessor(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff", const int GaugeWidth = 10, const double DrawStartPos = -25.0, const double NoUseArea = 50.0)
				: string(string), Val({ 0, 100 }), GraphInfo(FilePath, BackgroundColor, GaugeWidth, DrawStartPos, NoUseArea) {}
			void Draw(const int X, const int Y) const noexcept {
				this->GraphInfo.Draw(X, Y + this->string.get().StringSize, this->Val.GraphParameter.Get<double>());
			}
			void Update(const double New) {
				this->Val.Update(static_cast<int>(New));
			}
			void ApplyViewParameter() {
				this->Val.Apply();
			}
		};
		class TransferPercentManager {
		private:
			double Max;
			double Current;
		public:
			TransferPercentManager() = default;
			double Calc(const double& Transfer) {
				this->Current = Transfer;
				this->Max = std::max(this->Max, this->Current);
				return Transfer / this->Max;
			}
			double GetCurrent() const noexcept { return this->Current; }
		};
	};
	class Processor : private Base::ResponsePercentDataProcessor {
	private:
		std::string ProcessorName;
		int ProcessNum;
		inline int CalcStringLength() {
			char Buffer[CharBufferSize];
			sprintf_s(Buffer, CharBufferSize, "%d％", Base::ResponsePercentDataProcessor::Val.RealParameter.Get());
			return Base::ResponsePercentDataProcessor::string.get().GetLength(Buffer);
		}
	public:
		Processor(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), ProcessorName(), ProcessNum() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::string.get().Draw(X, Y, "CPU");
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& data) {
			if (this->ProcessorName.empty()) this->ProcessorName = data.at("name").get<std::string>();
			const double val = data.at("usage").get<double>();
			Base::ResponsePercentDataProcessor::Update(val);
			this->ProcessNum = static_cast<int>(data.at("process").get<double>());
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class Memory : private Base::ResponsePercentDataProcessor {
	private:
		double TotalMemory;
		double MemoryUsed;
	public:
		Memory(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), TotalMemory(), MemoryUsed() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& data) {
			Base::ResponsePercentDataProcessor::Update(data.at("usedper").get<double>());
			this->MemoryUsed = data.at("used").get<double>();
			this->TotalMemory = data.at("total").get<double>(); // 仮想メモリ全体の容量はシステムの状態によって変化することがあるから変更可能にしておく必要あり
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskUsage : private Base::ResponsePercentDataProcessor {
	private:
		double DiskUsedVal;
		double DiskTotal;
	public:
		DiskUsage(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), DiskTotal(), DiskUsedVal() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& diskused, const picojson::object& disktotal) {
			Base::ResponsePercentDataProcessor::Update(diskused.at("per").get<double>());
			this->DiskUsedVal = diskused.at("capacity").get<double>();
			this->DiskTotal = disktotal.at("capacity").get<double>();
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskRead : private Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
	public:
		DiskRead(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& DiskInfo) {
			Base::ResponsePercentDataProcessor::Update(this->Transfer.Calc(DiskInfo.at("read").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskWrite : private Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
	public:
		DiskWrite(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& DiskInfo) {
			Base::ResponsePercentDataProcessor::Update(this->Transfer.Calc(DiskInfo.at("write").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class NetworkReceive : private Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
	public:
		NetworkReceive(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& NetworkInfo) {
			Base::ResponsePercentDataProcessor::Update(this->Transfer.Calc(NetworkInfo.at("read").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class NetworkSend : private Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
	public:
		NetworkSend(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const noexcept {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
		void Update(const picojson::object& NetworkInfo) {
			Base::ResponsePercentDataProcessor::Update(this->Transfer.Calc(NetworkInfo.at("write").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	Processor processor;
	//Memory memory;
	//DiskUsage diskUsed;
	//DiskRead diskRead;
	//DiskWrite diskWrite;
	//NetworkReceive netReceive;
	//NetworkSend netSend;
public:
	ResponseProcessingManager(StringManager& string) :
		processor(string, ".\\Graph\\Processor.png") {}
	void Draw() const noexcept {
		this->processor.Draw(0, 0);
	}
	void Update(const picojson::object& obj) {
		this->processor.Update(obj.at("cpu").get<picojson::object>());
	}
	void ApplyViewParameter() {
		this->processor.ApplyViewParameter();
	}
};
