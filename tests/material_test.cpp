#include "modalith/material/material.hpp"

#include <gtest/gtest.h>

TEST(Sellmeier3, SchottBk7MatchesFraunhoferDLine) {
  modalith::MaterialCatalog catalog;
  const auto bk7 = catalog.find("n-bk7");
  EXPECT_NEAR(bk7->refractive_index(587.5618), 1.5168000345, 1.0e-9);
}

TEST(Sellmeier3, Bk7HasNormalVisibleDispersion) {
  modalith::MaterialCatalog catalog;
  const auto bk7 = catalog.find("N-BK7");
  EXPECT_GT(bk7->refractive_index(486.1327), bk7->refractive_index(656.2725));
}

TEST(MaterialCatalog, RejectsUnknownGlass) {
  modalith::MaterialCatalog catalog;
  EXPECT_THROW(static_cast<void>(catalog.find("UNOBTAINIUM")), std::out_of_range);
}
