#include "search/search_tests_support/test_search_engine.hpp"

#include "search/search_query.hpp"
#include "search/search_query_factory.hpp"
#include "search/suggest.hpp"

#include "indexer/categories_holder.hpp"
#include "indexer/scales.hpp"

#include "storage/country_info_getter.hpp"

#include "platform/platform.hpp"

#include "defines.hpp"

#include "std/unique_ptr.hpp"

namespace
{
class TestQuery : public search::Query
{
public:
  TestQuery(Index & index, CategoriesHolder const & categories,
            vector<search::Suggest> const & suggests, storage::CountryInfoGetter const & infoGetter)
    : search::Query(index, categories, suggests, infoGetter)
  {
  }

  // search::Query overrides:
  int GetQueryIndexScale(m2::RectD const & /* viewport */) const override
  {
    return scales::GetUpperScale();
  }
};

class TestSearchQueryFactory : public ::search::SearchQueryFactory
{
  // search::SearchQueryFactory overrides:
  unique_ptr<::search::Query> BuildSearchQuery(
      Index & index, CategoriesHolder const & categories,
      vector<::search::Suggest> const & suggests,
      storage::CountryInfoGetter const & infoGetter) override
  {
    return make_unique<TestQuery>(index, categories, suggests, infoGetter);
  }
};
}  // namespace

namespace search
{
namespace tests_support
{
TestSearchEngine::TestSearchEngine(Engine::Params const & params)
  : m_platform(GetPlatform())
  , m_infoGetter(storage::CountryInfoReader::CreateCountryInfoReader(m_platform))
  , m_engine(*this, GetDefaultCategories(), *m_infoGetter, make_unique<TestSearchQueryFactory>(),
             params)
{
}

TestSearchEngine::TestSearchEngine(unique_ptr<storage::CountryInfoGetter> infoGetter,
                                   Engine::Params const & params)
  : m_platform(GetPlatform())
  , m_infoGetter(move(infoGetter))
  , m_engine(*this, GetDefaultCategories(), *m_infoGetter, make_unique<TestSearchQueryFactory>(),
             params)
{
}

TestSearchEngine::TestSearchEngine(unique_ptr<storage::CountryInfoGetter> infoGetter,
                                   unique_ptr<::search::SearchQueryFactory> factory,
                                   Engine::Params const & params)
  : m_platform(GetPlatform())
  , m_infoGetter(move(infoGetter))
  , m_engine(*this, GetDefaultCategories(), *m_infoGetter, move(factory), params)
{
}

TestSearchEngine::TestSearchEngine(unique_ptr<::search::SearchQueryFactory> factory,
                                   Engine::Params const & params)
  : m_platform(GetPlatform())
  , m_infoGetter(storage::CountryInfoReader::CreateCountryInfoReader(m_platform))
  , m_engine(*this, GetDefaultCategories(), *m_infoGetter, move(factory), params)
{
}

TestSearchEngine::~TestSearchEngine() {}

weak_ptr<::search::QueryHandle> TestSearchEngine::Search(::search::SearchParams const & params,
                                                         m2::RectD const & viewport)
{
  return m_engine.Search(params, viewport);
}
}  // namespace tests_support
}  // namespace search
