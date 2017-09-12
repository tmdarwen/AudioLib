#include <gtest/gtest.h>
#include <Signal/PeakProfile.h>
#include <Signal/SignalConversion.h>
#include <Utilities/Exception.h>
#include <vector>
#include <memory>

// The fixture
class PeakProfileData : public testing::Test
{
	public:
		Signal::FrequencyDomain frequencyDomain_;
		std::unique_ptr<Signal::PeakProfile> peakProfile_;

		PeakProfileData()
		{
			// Create a ficticious frequency domain representation with a couple of peaks
			frequencyDomain_.PushFrequencyBin({0.0, 0.0});
			frequencyDomain_.PushFrequencyBin({0.1, 0.1});
			frequencyDomain_.PushFrequencyBin({0.2, 0.2});
			frequencyDomain_.PushFrequencyBin({1.5, 1.5});
			frequencyDomain_.PushFrequencyBin({0.6, 0.6});
			frequencyDomain_.PushFrequencyBin({0.4, 0.4});
			frequencyDomain_.PushFrequencyBin({0.35, 0.35});
			frequencyDomain_.PushFrequencyBin({0.3, 0.3});
			frequencyDomain_.PushFrequencyBin({0.2, 0.2});
			frequencyDomain_.PushFrequencyBin({0.1, 0.1});
			frequencyDomain_.PushFrequencyBin({0.06, 0.06});
			frequencyDomain_.PushFrequencyBin({0.7, 0.7});
			frequencyDomain_.PushFrequencyBin({2.4, 2.4});
			frequencyDomain_.PushFrequencyBin({0.7, 0.7});
			frequencyDomain_.PushFrequencyBin({0.3, 0.3});
			frequencyDomain_.PushFrequencyBin({0.1, 0.1});

			peakProfile_.reset(new Signal::PeakProfile(frequencyDomain_));
		}
};

TEST_F(PeakProfileData, TestPeakValues)
{
	// Test that we get the proper peak values back for each bin
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(0));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(1));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(2));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(3));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(4));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(5));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(6));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(7));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(8));
	EXPECT_EQ(3, peakProfile_->GetLocalPeakForBin(9));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(10));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(11));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(12));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(13));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(14));
	EXPECT_EQ(12, peakProfile_->GetLocalPeakForBin(15));

	auto peakBins{peakProfile_->GetAllPeakBins()};
	EXPECT_EQ(2, peakBins.size());
	EXPECT_EQ(3, peakBins[0]);
	EXPECT_EQ(12, peakBins[1]);

}

TEST_F(PeakProfileData, TestGettingValleyBins)
{
	// Test getting the valleys for peak bin 3
	{
		auto valleyBins{peakProfile_->GetValleyBins(3)};
		EXPECT_EQ(0, valleyBins.first);
		EXPECT_EQ(10, valleyBins.second);
	}

	// Test getting the valleys for peak bin 12
	{
		auto valleyBins{peakProfile_->GetValleyBins(12)};
		EXPECT_EQ(10, valleyBins.first);
		EXPECT_EQ(15, valleyBins.second);
	}
}

TEST_F(PeakProfileData, TestGettingValleyBinsFailure)
{
	// Test getting the valleys for peak bin 3
	EXPECT_THROW(peakProfile_->GetValleyBins(4), Utilities::Exception);
}

TEST(PeakProfileTests, PeakProfileTest)
{
	// Create a ficticious frequency domain representation with a couple of peaks
	Signal::FrequencyDomain frequencyDomain;
	frequencyDomain.PushFrequencyBin({0.0, 0.0});
	frequencyDomain.PushFrequencyBin({0.1, 0.1});
	frequencyDomain.PushFrequencyBin({0.2, 0.2});
	frequencyDomain.PushFrequencyBin({1.5, 1.5});
	frequencyDomain.PushFrequencyBin({0.6, 0.6});
	frequencyDomain.PushFrequencyBin({0.4, 0.4});
	frequencyDomain.PushFrequencyBin({0.35, 0.35});
	frequencyDomain.PushFrequencyBin({0.3, 0.3});
	frequencyDomain.PushFrequencyBin({0.2, 0.2});
	frequencyDomain.PushFrequencyBin({0.1, 0.1});
	frequencyDomain.PushFrequencyBin({0.06, 0.06});
	frequencyDomain.PushFrequencyBin({0.7, 0.7});
	frequencyDomain.PushFrequencyBin({2.4, 2.4});
	frequencyDomain.PushFrequencyBin({0.7, 0.7});
	frequencyDomain.PushFrequencyBin({0.3, 0.3});
	frequencyDomain.PushFrequencyBin({0.1, 0.1});

	Signal::PeakProfile peakProfile{frequencyDomain};

	// Test that we get the proper peak values back for each bin
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(0));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(1));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(2));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(3));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(4));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(5));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(6));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(7));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(8));
	EXPECT_EQ(3, peakProfile.GetLocalPeakForBin(9));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(10));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(11));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(12));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(13));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(14));
	EXPECT_EQ(12, peakProfile.GetLocalPeakForBin(15));

	auto peakBins{peakProfile.GetAllPeakBins()};
	EXPECT_EQ(2, peakBins.size());
	EXPECT_EQ(3, peakBins[0]);
	EXPECT_EQ(12, peakBins[1]);
}
