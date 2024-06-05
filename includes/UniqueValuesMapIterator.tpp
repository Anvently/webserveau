#ifndef UNIQUE_VALUE_MAP_ITERATOR
# define UNIQUE_VALUE_MAP_ITERATOR

#include <map>
#include <set>
#include <list>
#include <algorithm>

template <typename Key, typename Value>
class UniqueValuesMapIterator_const
{

	private:

		UniqueValuesMapIterator_const(void) : _baseIt(), _uniqueValues() {}
		typename std::map<Key, Value>::const_iterator	_baseIt;
		std::set<Value>									_uniqueValues;

	public:

		UniqueValuesMapIterator_const(typename std::map<Key, Value>::const_iterator it) \
			: _baseIt(it), _uniqueValues() {}
		UniqueValuesMapIterator_const(const UniqueValuesMapIterator_const& copy) : \
			_baseIt(copy._baseIt), _uniqueValues(copy._uniqueValues) {}
		UniqueValuesMapIterator_const&	operator=(const UniqueValuesMapIterator_const& copy) {
			_baseIt = copy._baseIt;
			_uniqueValues = copy._uniqueValues;
		}
		UniqueValuesMapIterator_const&	operator=(const typename std::map<Key, Value>::const_iterator& copy) {
			_baseIt = copy;
			_uniqueValues.clear();
		}
		const typename std::pair<const Key, Value>&	operator*(void) const {
			return (*_baseIt);
		}
		const typename std::pair<const Key, Value>*	operator->(void) {
			return (_baseIt.operator->());
		}
		bool	operator==(const UniqueValuesMapIterator_const& rhi) const {
			return (_baseIt == rhi._baseIt);
		}
		bool	operator!=(const UniqueValuesMapIterator_const& rhi) const {
			return (_baseIt != rhi._baseIt);
		}
		bool	operator==(const typename std::map<Key, Value>::const_iterator& rhi) const {
			return (_baseIt == rhi);
		}
		bool	operator!=(const typename std::map<Key, Value>::const_iterator& rhi) const {
			return (_baseIt != rhi);
		}
		UniqueValuesMapIterator_const&	operator++(void) {
			_uniqueValues.insert(_uniqueValues.end(), _baseIt->second);
			do {
				++_baseIt;
			} while (std::find(_uniqueValues.begin(), _uniqueValues.end(), _baseIt->second) != _uniqueValues.end());
			return (*this);
		};
		UniqueValuesMapIterator_const&	operator--(void) {
			_uniqueValues.insert(_uniqueValues.end(), _baseIt->second);
			do {
				--_baseIt;
			} while (std::find(_uniqueValues.begin(), _uniqueValues.end(), _baseIt->second) != _uniqueValues.end());
			return (*this);
		};
		UniqueValuesMapIterator_const	operator++(int) {
			UniqueValuesMapIterator_const	tmp(*this);
			++(*this);
			return (tmp);
		};
		UniqueValuesMapIterator_const	operator--(int) {
			UniqueValuesMapIterator_const	tmp(*this);
			--(*this);
			return (tmp);
		};
};

template <typename Key, typename Value>
class UniqueValuesMapIterator
{

	private:

		UniqueValuesMapIterator(void) : _baseIt(), _uniqueValues(){}
		typename std::map<Key, Value>::iterator	_baseIt;
		std::set<Value>						_uniqueValues;

	public:

		UniqueValuesMapIterator(typename std::map<Key, Value>::iterator it) \
			: _baseIt(it), _uniqueValues() {}
		UniqueValuesMapIterator(const UniqueValuesMapIterator& copy) : \
			_baseIt(copy._baseIt), _uniqueValues(copy._uniqueValues) {}
		UniqueValuesMapIterator&	operator=(const UniqueValuesMapIterator& copy) {
			_baseIt = copy._baseIt;
			_uniqueValues = copy._uniqueValues;
		}
		UniqueValuesMapIterator&	operator=(const typename std::map<Key, Value>::iterator& copy) {
			_baseIt = copy;
			_uniqueValues.clear();
		}
		typename std::pair<const Key, Value>&	operator*(void) const {
			return (*_baseIt);
		}
		typename std::pair<const Key, Value>*	operator->(void) {
			return (_baseIt.operator->());
		}
		bool	operator==(const UniqueValuesMapIterator& rhi) const {
			return (_baseIt == rhi._baseIt);
		}
		bool	operator!=(const UniqueValuesMapIterator& rhi) const {
			return (_baseIt != rhi._baseIt);
		}
		bool	operator==(const typename std::map<Key, Value>::iterator& rhi) const {
			return (_baseIt == rhi);
		}
		bool	operator!=(const typename std::map<Key, Value>::iterator& rhi) const {
			return (_baseIt != rhi);
		}
		UniqueValuesMapIterator&	operator++(void) {
			_uniqueValues.insert(_uniqueValues.end(), _baseIt->second);
			do {
				++_baseIt;
			} while (std::find(_uniqueValues.begin(), _uniqueValues.end(), _baseIt->second) != _uniqueValues.end());
			return (*this);
		};
		UniqueValuesMapIterator&	operator--(void) {
			_uniqueValues.insert(_uniqueValues.end(), _baseIt->second);
			do {
				--_baseIt;
			} while (std::find(_uniqueValues.begin(), _uniqueValues.end(), _baseIt->second) != _uniqueValues.end());
			return (*this);
		};
		UniqueValuesMapIterator	operator++(int) {
			UniqueValuesMapIterator	tmp(*this);
			++(*this);
			return (tmp);
		};
		UniqueValuesMapIterator	operator--(int) {
			UniqueValuesMapIterator	tmp(*this);
			--(*this);
			return (tmp);
		};
};

#endif
