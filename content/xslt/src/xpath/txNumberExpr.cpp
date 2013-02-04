/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/FloatingPoint.h"

#include "txExpr.h"
#include <math.h>
#include "txIXPathContext.h"

nsresult
txNumberExpr::evaluate(txIEvalContext* aContext, txAExprResult** aResult)
{
    *aResult = nullptr;

    nsRefPtr<txAExprResult> exprRes;
    nsresult rv = mRightExpr->evaluate(aContext, getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    double rightDbl = exprRes->numberValue();

    rv = mLeftExpr->evaluate(aContext, getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    double leftDbl = exprRes->numberValue();
    double result = 0;

    switch (mOp) {
        case ADD:
            result = leftDbl + rightDbl;
            break;

        case SUBTRACT:
            result = leftDbl - rightDbl;
            break;

        case DIVIDE:
            if (rightDbl == 0) {
#if defined(XP_WIN)
                /* XXX MSVC miscompiles such that (NaN == 0) */
                if (MOZ_DOUBLE_IS_NaN(rightDbl))
                    result = MOZ_DOUBLE_NaN();
                else
#endif
                if (leftDbl == 0 || MOZ_DOUBLE_IS_NaN(leftDbl))
                    result = MOZ_DOUBLE_NaN();
                else if (MOZ_DOUBLE_IS_NEGATIVE(leftDbl) ^ MOZ_DOUBLE_IS_NEGATIVE(rightDbl))
                    result = MOZ_DOUBLE_NEGATIVE_INFINITY();
                else
                    result = MOZ_DOUBLE_POSITIVE_INFINITY();
            }
            else
                result = leftDbl / rightDbl;
            break;

        case MODULUS:
            if (rightDbl == 0) {
                result = MOZ_DOUBLE_NaN();
            }
            else {
#if defined(XP_WIN)
                /* Workaround MS fmod bug where 42 % (1/0) => NaN, not 42. */
                if (!MOZ_DOUBLE_IS_INFINITE(leftDbl) && MOZ_DOUBLE_IS_INFINITE(rightDbl))
                    result = leftDbl;
                else
#endif
                result = fmod(leftDbl, rightDbl);
            }
            break;

        case MULTIPLY:
            result = leftDbl * rightDbl;
            break;
    }

    return aContext->recycler()->getNumberResult(result, aResult);
} //-- evaluate

TX_IMPL_EXPR_STUBS_2(txNumberExpr, NUMBER_RESULT, mLeftExpr, mRightExpr)

bool
txNumberExpr::isSensitiveTo(ContextSensitivity aContext)
{
    return mLeftExpr->isSensitiveTo(aContext) ||
           mRightExpr->isSensitiveTo(aContext);
}

#ifdef TX_TO_STRING
void
txNumberExpr::toString(nsAString& str)
{
    mLeftExpr->toString(str);

    switch (mOp) {
        case ADD:
            str.AppendLiteral(" + ");
            break;
        case SUBTRACT:
            str.AppendLiteral(" - ");
            break;
        case DIVIDE:
            str.AppendLiteral(" div ");
            break;
        case MODULUS:
            str.AppendLiteral(" mod ");
            break;
        case MULTIPLY:
            str.AppendLiteral(" * ");
            break;
    }

    mRightExpr->toString(str);

}
#endif
