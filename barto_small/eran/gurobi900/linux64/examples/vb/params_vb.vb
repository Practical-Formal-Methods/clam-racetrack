' Copyright 2019, Gurobi Optimization, LLC */

' Use parameters that are associated with a model.

' A MIP is solved for a few seconds with different sets of parameters.
' The one with the smallest MIP gap is selected, and the optimization
' is resumed until the optimal solution is found.

Imports System
Imports Gurobi

Class params_vb
    Shared Sub Main(args As String())
        If args.Length < 1 Then
            Console.Out.WriteLine("Usage: params_vb filename")
            Return
        End If

        Try
            ' Read model and verify that it is a MIP
            Dim env As New GRBEnv()
            Dim m As New GRBModel(env, args(0))
            If m.IsMIP = 0 Then
                Console.WriteLine("The model is not an integer program")
                Environment.Exit(1)
            End If

            ' Set a 2 second time limit
            m.Parameters.TimeLimit = 2.0

            ' Now solve the model with different values of MIPFocus
            Dim bestModel As New GRBModel(m)
            bestModel.Optimize()
            For i As Integer = 1 To 3
                m.Reset()
                m.Parameters.MIPFocus = i
                m.Optimize()
                If bestModel.MIPGap > m.MIPGap Then
                    Dim swap As GRBModel = bestModel
                    bestModel = m
                    m = swap
                End If
            Next

            ' Finally, delete the extra model, reset the time limit and
            ' continue to solve the best model to optimality
            m.Dispose()
            bestModel.Parameters.TimeLimit = GRB.INFINITY
            bestModel.Optimize()

            Console.WriteLine("Solved with MIPFocus: " & bestModel.Parameters.MIPFocus)
        Catch e As GRBException
            Console.WriteLine("Error code: " + e.ErrorCode & ". " + e.Message)
        End Try
    End Sub
End Class
