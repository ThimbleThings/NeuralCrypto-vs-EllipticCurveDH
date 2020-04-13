#include "NC.h"
#include <random>

Eigen::IOFormat NCFmt(Eigen::FullPrecision, Eigen::DontAlignCols, ";", ";", "", "", "", "");

NC::NC(uint32_t k, uint32_t n, int32_t l, uint32_t roundThreshold, std::string learnRule)
{
	m_k = k;
	m_n = n;
	m_l = l;
	m_tau = 0;
	m_roundThreshold = roundThreshold;
	m_learnRule = learnRule;

	std::random_device rd;								 // Will be used to obtain a seed for the random number engine
	std::mt19937 generator(rd());						 // Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<int> w_dis(-m_l, m_l); // Initialize uniform distribution for randomized weights
	std::uniform_int_distribution<int> i_dis(0, 1);		 // Initialize uniform distribution for randomized inputs
	uni = i_dis;										 // Store in uni, since initializing uni does not work
	gen = generator;									 // Store in gen, since initializing gen does not work
	
	// Generate a random weight vector based on m_k, m_n, and m_l (m_l the range for w_dis)
	m_weights = Eigen::VectorXi::Zero(m_n * m_k).unaryExpr([&](int dummy) { return w_dis(gen); });

	// Initialize the size for the sigma vector
	m_sigmas = Eigen::VectorXi(m_k);

	// Initialize round
	m_round = 0;

	// Initialize the function pointer UpdateRule
	if (m_learnRule.compare("hebbian") == 0)
	{
		UpdateRule = &NC::Hebbian;
	}
	else if (m_learnRule.compare("anti_hebbian") == 0)
	{
		UpdateRule = &NC::AntiHebbian;
	}
	else
	{
		if (learnRule.compare("random_walk") != 0)
		{
			std::cout << "Unknown learning rule: " << learnRule << ". Default to random_walk\n";
		}
		UpdateRule = &NC::RandomWalk;
	}
}

Eigen::VectorXi NC::GenerateInputVector()
{
	// Generate a random input vector based on m_k and m_n
	// Since we want the parameters to be x_i in {-1, +1} do this:

	Eigen::VectorXi inVector = Eigen::VectorXi::Zero(m_n * m_k).unaryExpr([&](int dummy) { return (uni(gen) * 2 - 1); });

	return inVector;
}

Eigen::VectorXi NC::VectorFromString(std::string &str_Vector)
{
	// replace ';' with ' ' in the string
	std::transform(str_Vector.begin(), str_Vector.end(), str_Vector.begin(), [](char c) { return c == ';' ? ' ' : c; });

	// create vector of ints from stringstream
	std::stringstream ss_Vector(str_Vector);
	std::vector<int> elements((std::istream_iterator<int>(ss_Vector)), std::istream_iterator<int>());

	// Create the Eigen::VectorXi
	Eigen::Map<Eigen::VectorXi> vector(elements.data(), elements.size());

	return vector;
}

int8_t NC::CalculateOutput(Eigen::VectorXi &inVector)
{
	// Calculates the output of the network

	// store last inVector for use in UpdateWeights (aka the called learning rule)
	m_inVector = inVector;

	// Calculate the hadamard product of the input vector and the weights
	Eigen::MatrixXi weighting = m_inVector.array() * m_weights.array();

	// Reshape the matrix
	weighting.resize(m_n, m_k);

	// Calculate the columnwise sums and of those the signs
	for (size_t i = 0; i < m_k; i++)
	{
		if (weighting.col(i).sum() >= 0)
		{
			m_sigmas(i) = 1;
		}
		else
		{
			m_sigmas(i) = -1;
		}
	}

	// Calculate the product of the sigmas
	m_tau = m_sigmas.prod();

	return m_tau;
}

void NC::UpdateWeights(int8_t partnerTau)
{
	// Use m_tau and m_learnRule to decide which function to call
	// Updates the weights according to the specified update rule.
	// partnerTau - Output bit from the other machine;

	if (m_tau == partnerTau)
	{
		// Do not need to pass partnerTau, since we know it is identical to m_tau
		(this->*UpdateRule)();

		// Increase round only when TPMs update
		m_round += 1;
	}
}

uint32_t NC::GetCurrentRound()
{
	return m_round;
}

void NC::GenerateKey()
{
	// The ECDH Code produces 32 bytes for the key (aka 256 bit)
	// Use SHA256 from CryptoPP to generate a 256 bit hash as key

	CryptoPP::SHA256 hash;

	// This is most certainly not clean...
	// Reinterpret the underlying integer array as a byte array,
	// and multiply the size of the integer array by the size of an int
	hash.Update((const byte *)m_weights.data(), m_weights.size() * sizeof(int));

	// Create a CryptoPP SecByteBlock
	CryptoPP::SecByteBlock secret(hash.DigestSize());

	// Store the hash as the secret key
	hash.Final((byte *)&secret[0]);

	m_secret = secret;
}

CryptoPP::SecByteBlock NC::GetSecret()
{
	return m_secret;
}

void NC::EnforceBounds()
{
	// i the index of the weight in the neurons weight vector
	for (size_t i = 0; i < m_n; i++)
	{
		// j the index of the neuron
		for (size_t j = 0; j < m_k; j++)
		{
			if (m_weights(i + j * m_n) > m_l)
			{
				m_weights(i + j * m_n) = m_l;
			}
			else if (m_weights(i + j * m_n) < -m_l)
			{
				m_weights(i + j * m_n) = -m_l;
			}
		}
	}
}

void NC::Hebbian()
{
	// i the index of the weight in the neurons weight vector
	for (size_t i = 0; i < m_n; i++)
	{
		// j the index of the neuron
		for (size_t j = 0; j < m_k; j++)
		{
			// Adjust the weight according to the learning rule
			// Move in steps of m_n (the size of the vectors for the hidden neurons)
			m_weights(i + j * m_n) += m_inVector(i + j * m_n) * m_tau * Theta(m_sigmas(j), m_tau);
		}
	}

	// Adjust the weights to be within bounds
	EnforceBounds();

	std::cout << "Hebbian, updated to: " << m_weights.format(NCFmt) << "\n";
}

void NC::AntiHebbian()
{
	// i the index of the weight in the neurons weight vector
	for (size_t i = 0; i < m_n; i++)
	{
		// j the index of the neuron
		for (size_t j = 0; j < m_k; j++)
		{
			// Adjust the weight according to the learning rule
			// Move in steps of m_n (the size of the vectors for the hidden neurons)
			m_weights(i + j * m_n) -= m_inVector(i + j * m_n) * m_tau * Theta(m_sigmas(j), m_tau);
		}
	}

	// Adjust the weights to be within bounds
	EnforceBounds();

	std::cout << "Anti-Hebbian, updated to: " << m_weights.format(NCFmt) << "\n";
}

void NC::RandomWalk()
{
	// i the index of the weight in the neurons weight vector
	for (size_t i = 0; i < m_n; i++)
	{
		// j the index of the neuron
		for (size_t j = 0; j < m_k; j++)
		{
			// Adjust the weight according to the learning rule
			// Move in steps of m_n (the size of the vectors for the hidden neurons)
			m_weights(i + j * m_n) += m_inVector(i + j * m_n) * Theta(m_sigmas(j), m_tau);
		}
	}

	// Adjust the weights to be within bounds
	EnforceBounds();

	std::cout << "Random-Walk, updated to: " << m_weights.format(NCFmt) << "\n";
}

bool NC::Theta(int tau1, int tau2)
{
	return tau1 == tau2;
}
