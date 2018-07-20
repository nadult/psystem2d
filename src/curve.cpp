#include "curve.h"

template <class T> Curve<T>::Curve() = default;
template <class T> Curve<T>::~Curve() = default;

template <class T>
Curve<T>::Curve(T value) : m_keys({0.0f}), m_values({value}), m_interp(InterpType::linear) {}

template <class T>
Curve<T>::Curve(vector<T> values, InterpType interp) : m_values(move(values)), m_interp(interp) {
	m_keys.resize(m_values.size());
	for(int n = 0; n < m_keys.size(); n++)
		m_keys[n] = float(n) / float(m_keys.size() - 1);
}

template <class T>
Curve<T>::Curve(vector<float> keys, vector<T> values, InterpType interp)
	: m_keys(move(keys)), m_values(move(values)), m_interp(interp) {
	DASSERT(m_keys.size() == m_values.size());
	for(int n = 0; n < m_keys.size(); n++) {
		PASSERT(m_keys[n] >= 0.0f && m_keys[n] <= 1.0f);
		if(n > 0)
			PASSERT(m_keys[n] >= m_keys[n - 1]);
	}
}

// TODO: add sampleLooped
template <class T> T Curve<T>::sample(float position) const {
	PASSERT(position >= 0.0f && position <= 1.0f);
	if(m_values.size() <= 1)
		return m_values.empty() ? T() : m_values.front();

	int id = 0;
	while(id < m_keys.size() && m_keys[id] < position)
		id++;
	id = id == 0 ? 0 : id - 1;

	int ids[4] = {id == 0 ? 0 : id - 1, id, id + 1, id + 2};

	if(ids[2] == m_keys.size())
		ids[2] = ids[3] = id;
	else if(ids[3] == m_keys.size())
		ids[3] = id + 1;

	float key1 = m_keys[ids[1]], key2 = m_keys[ids[2]];
	float t = key1 == key2 ? 0.0 : (position - key1) / (key2 - key1);

	switch(m_interp) {
	case InterpType::linear:
		return lerp(m_values[ids[1]], m_values[ids[2]], t);
	case InterpType::cosine:
		return interpCosine(m_values[ids[1]], m_values[ids[2]], t);
	case InterpType::quadratic:
		return interpQuadratic(m_values[ids[0]], m_values[ids[1]], m_values[ids[2]],
							   m_values[ids[3]], t);
	case InterpType::cubic:
		return interpCubic(m_values[ids[0]], m_values[ids[1]], m_values[ids[2]], m_values[ids[3]],
						   t);
	}

	return T();
}

template struct Curve<float>;
template struct Curve<float2>;
template struct Curve<float3>;
